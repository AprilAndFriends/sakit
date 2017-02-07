/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdlib.h>

#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "HttpResponse.h"
#include "HttpSocket.h"
#include "HttpSocketThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "SocketDelegate.h"
#include "State.h"

namespace sakit
{
	HttpSocketThread::HttpSocketThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency)
	{
		this->name = "SAKit HTTP Socket";
		this->stream = new hstream();
		this->response = new HttpResponse();
	}

	HttpSocketThread::~HttpSocketThread()
	{
		delete this->stream;
		delete this->response;
	}

	void HttpSocketThread::_updateConnect()
	{
		Host localHost;
		unsigned short localPort = 0;
		if (!this->socket->isConnected() && !this->socket->connect(this->host, this->port, localHost, localPort, *this->timeout, *this->retryFrequency))
		{
			hmutex::ScopeLock lock(&this->mutex);
			this->result = FAILED;
			this->executing = false;
		}
	}

	void HttpSocketThread::_updateSend()
	{
		int sent = 0;
		int count = (int)this->stream->size();
		while (this->isRunning() && this->executing)
		{
			if (!this->socket->send(this->stream, count, sent))
			{
				hmutex::ScopeLock lock(&this->mutex);
				this->result = FAILED;
				lock.release();
				this->executing = false;
				this->socket->disconnect();
				break;
			}
			if (this->stream->eof())
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		this->stream->clear();
	}

	void HttpSocketThread::_updateReceive()
	{
		float time = 0.0f;
		int64_t size = 0;
		int64_t lastSize = 0;
		while (this->isRunning() && this->executing)
		{
			if (!this->socket->receive(this->response, this->mutex))
			{
				break;
			}
			if (this->response->headersComplete && this->response->bodyComplete)
			{
				break;
			}
			size = this->response->raw.size();
			if (lastSize != size)
			{
				lastSize = size;
				// retry attempts are reset after a successful read
				time = 0.0f;
				continue;
			}
			time += *this->retryFrequency;
			if (time >= *this->timeout)
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		// if timed out, has no predefined length, all headers were received and there is a body
		if (time >= *this->timeout && !this->response->headers.hasKey(SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH) && this->response->headersComplete && this->response->body.size() > 0)
		{
			if (!this->response->headers.hasKey(SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH) && this->response->body.size() > 0)
			{
				// let's say it's complete, we don't know its supposed length anyway
				hlog::warn(logTag, "HttpSocket did not return header " SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH "! Body might be incomplete, but will be considered complete.");
				this->response->bodyComplete = true;
			}
			else if ((int)this->response->headers[SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH] == 0) // empty body
			{
				this->response->bodyComplete = true;
			}
		}
		hmutex::ScopeLock lock(&this->mutex);
		// only a response with complete headers and a complete body is considered
		if (this->response->headersComplete && this->response->bodyComplete)
		{
			this->result = FINISHED;
		}
		else
		{
			this->result = FAILED;
			this->response->clear();
			this->socket->disconnect();
		}
	}

	void HttpSocketThread::_updateProcess()
	{
		this->_updateConnect();
		if (this->isRunning() && this->executing)
		{
			this->_updateSend();
		}
		if (this->isRunning() && this->executing)
		{
			this->_updateReceive();
		}
	}

}
