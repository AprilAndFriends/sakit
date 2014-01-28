/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdlib.h>

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "HttpResponse.h"
#include "HttpSocketThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "SocketDelegate.h"
#include "State.h"

namespace sakit
{
	HttpSocketThread::HttpSocketThread(PlatformSocket* socket) : WorkerThread(socket)
	{
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
		if (!this->socket->isConnected() && !this->socket->connect(this->host, this->port))
		{
			this->mutex.lock();
			this->result = FAILED;
			this->mutex.unlock();
			this->running = false;
		}
	}

	void HttpSocketThread::_updateSend()
	{
		int sent = 0;
		int count = this->stream->size();
		while (this->running)
		{
			if (!this->socket->send(this->stream, count, sent))
			{
				this->mutex.lock();
				this->result = FAILED;
				this->mutex.unlock();
				this->running = false;
				this->socket->disconnect();
				break;
			}
			if (this->stream->eof())
			{
				break;
			}
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		this->stream->clear();
	}

	void HttpSocketThread::_updateReceive()
	{
		int retryAttempts = sakit::getRetryAttempts();
		int size = 0;
		int lastSize = 0;
		while (this->running && retryAttempts > 0)
		{
			if (!this->socket->receive(this->response, this->mutex))
			{
				break;
			}
			if (this->response->HeadersComplete && this->response->BodyComplete)
			{
				break;
			}
			size = this->response->Raw.size();
			if (lastSize != size)
			{
				lastSize = size;
				// retry attempts are reset after a successful read
				retryAttempts = sakit::getRetryAttempts();
				continue;
			}
			retryAttempts--;
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		// if timed out, has no predefined length, all headers were received and there is a body
		if (retryAttempts == 0 && !this->response->Headers.has_key("Content-Length") && this->response->HeadersComplete && this->response->Body.size() > 0)
		{
			// let's say it's complete, we don't know its supposed length anyway
			this->response->BodyComplete = true;
		}
		this->mutex.lock();
		if (this->response->Raw.size() > 0)
		{
			this->result = FINISHED;
			this->response->Raw.rewind();
			this->response->Body.rewind();
		}
		else
		{
			this->result = FAILED;
			this->response->clear();
			this->socket->disconnect();
		}
		this->mutex.unlock();
	}

	void HttpSocketThread::_updateProcess()
	{
		this->_updateConnect();
		if (this->running)
		{
			this->_updateSend();
		}
		if (this->running)
		{
			this->_updateReceive();
		}
	}

}
