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
			hmutex::ScopeLock lock(&this->resultMutex);
			this->result = State::Failed;
			lock.release();
			this->executing = false;
		}
	}

	void HttpSocketThread::_updateSend()
	{
		int sentCount = 0;
		int count = (int)this->stream->size();
		while (this->isRunning() && this->executing)
		{
			if (!this->socket->send(this->stream, count, sentCount))
			{
				hmutex::ScopeLock lock(&this->resultMutex);
				this->result = State::Failed;
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
		hmutex::ScopeLock lock;
		int maxCount = 0;
		hstream stream(maxCount);
		float time = 0.0f;
		int64_t size = 0LL;
		int64_t lastSize = 0LL;
		int64_t position = 0LL;
		bool hasMoreData = true;
		// this implementation differs slightly from HttpSocket::_receiveHttpDirect() due to required mutex locking
		while (this->isRunning() && this->executing)
		{
			maxCount = HTTP_SOCKET_THREAD_BUFFER_SIZE;
			if (!this->socket->receive(&stream, maxCount))
			{
				if (stream.size() > 0)
				{
					stream.rewind();
					lock.acquire(&this->responseMutex);
					this->response->raw.seek(0, hseek::End);
					position = this->response->raw.position();
					this->response->raw.writeRaw(stream);
					this->response->raw.seek(position, hseek::Start);
					this->response->parseFromRaw();
					lock.release();
				}
				break;
			}
			stream.rewind();
			lock.acquire(&this->responseMutex);
			this->response->raw.seek(0, hseek::End);
			position = this->response->raw.position();
			this->response->raw.writeRaw(stream);
			this->response->raw.seek(position, hseek::Start);
			this->response->parseFromRaw();
			size = this->response->raw.size();
			if (this->response->headersComplete && this->response->bodyComplete)
			{
				lock.release();
				break;
			}
			lock.release();
			stream.clear(maxCount);
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
		lock.acquire(&this->responseMutex);
		// if timed out, has no predefined length, all headers were received and there is a body
		if (time >= *this->timeout && !this->response->headers.hasKey(SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH) && this->response->headersComplete && this->response->body.size() > 0)
		{
			if (!this->response->headers.hasKey(SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH) && this->response->body.size() > 0)
			{
				// let's say it's complete, we don't know its supposed length anyway
				hlog::warn(logTag, "HttpSocket did not return header '" SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH "'! Body might be incomplete, but will be considered complete.");
				this->response->bodyComplete = true;
			}
			else if ((int)this->response->headers[SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH] == 0) // empty body
			{
				this->response->bodyComplete = true;
			}
		}
		bool completeHeaders = (this->response->headersComplete && this->response->bodyComplete);
		if (!completeHeaders)
		{
			this->response->clear();
		}
		lock.release();
		lock.acquire(&this->resultMutex);
		// only a response with complete headers and a complete body is considered
		if (completeHeaders)
		{
			this->result = State::Finished;
		}
		else
		{
			this->result = State::Failed;
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
