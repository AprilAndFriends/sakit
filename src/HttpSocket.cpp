/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "HttpResponse.h"
#include "HttpSocket.h"
#include "HttpSocketDelegate.h"
#include "HttpSocketThread.h"
#include "PlatformSocket.h"
#include "sakit.h"

#define NORMAL_EXECUTE(name, constant) \
	bool HttpSocket::execute ## name(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethod(response, SAKIT_HTTP_REQUEST_ ## constant, url, customHeaders); \
	}
#define NORMAL_EXECUTE_ASYNC(name, constant) \
	bool HttpSocket::execute ## name ## Async(Url url, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethodAsync(SAKIT_HTTP_REQUEST_ ## constant, url, customHeaders); \
	}
#define CONNECTED_EXECUTE(name, constant) \
	bool HttpSocket::execute ## name(HttpResponse* response, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethod(response, SAKIT_HTTP_REQUEST_ ## constant, customHeaders); \
	}
#define CONNECTED_EXECUTE_ASYNC(name, constant) \
	bool HttpSocket::execute ## name ## Async(hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethodAsync(SAKIT_HTTP_REQUEST_ ## constant, customHeaders); \
	}

namespace sakit
{
	unsigned short HttpSocket::DefaultPort = 80;

	HttpSocket::HttpSocket(HttpSocketDelegate* socketDelegate, Protocol protocol) : SocketBase(), keepAlive(false)
	{
		this->socketDelegate = socketDelegate;
		this->protocol = protocol;
		this->port = HttpSocket::DefaultPort;
		this->socket->setConnectionLess(false);
		this->thread = new HttpSocketThread(this->socket);
		this->__register();
	}

	HttpSocket::~HttpSocket()
	{
		this->__unregister();
		this->thread->running = false;
		this->thread->join();
		delete this->thread;
	}

	bool HttpSocket::isConnected()
	{
		return this->url.isValid();
	}

	bool HttpSocket::isExecuting()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == RUNNING);
		this->thread->mutex.unlock();
		return result;
	}

	void HttpSocket::update(float timeSinceLastFrame)
	{
		this->thread->mutex.lock();
		WorkerThread::Result result = this->thread->result;
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			this->thread->mutex.unlock();
			return;
		}
		this->thread->state = IDLE;
		this->thread->result = WorkerThread::IDLE;
		HttpResponse* response = this->thread->response;
		this->thread->response = new HttpResponse();
		this->thread->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			this->socketDelegate->onExecuteCompleted(this, response, this->url);
		}
		else if (result == WorkerThread::FAILED)
		{
			this->socketDelegate->onExecuteFailed(this, response, this->url);
		}
		delete response;
		if (!this->keepAlive || response->Headers.try_get_by_key("Connection", "") == "close" || !this->socket->isConnected())
		{
			this->_terminateConnection();
		}
	}

	NORMAL_EXECUTE(Options, OPTIONS);
	NORMAL_EXECUTE(Get, GET);
	NORMAL_EXECUTE(Head, HEAD);
	NORMAL_EXECUTE(Post, POST);
	NORMAL_EXECUTE(Put, PUT);
	NORMAL_EXECUTE(Delete, DELETE);
	NORMAL_EXECUTE(Trace, TRACE);
	NORMAL_EXECUTE(Connect, CONNECT);

	NORMAL_EXECUTE_ASYNC(Options, OPTIONS);
	NORMAL_EXECUTE_ASYNC(Get, GET);
	NORMAL_EXECUTE_ASYNC(Head, HEAD);
	NORMAL_EXECUTE_ASYNC(Post, POST);
	NORMAL_EXECUTE_ASYNC(Put, PUT);
	NORMAL_EXECUTE_ASYNC(Delete, DELETE);
	NORMAL_EXECUTE_ASYNC(Trace, TRACE);
	NORMAL_EXECUTE_ASYNC(Connect, CONNECT);

	CONNECTED_EXECUTE(Options, OPTIONS);
	CONNECTED_EXECUTE(Get, GET);
	CONNECTED_EXECUTE(Head, HEAD);
	CONNECTED_EXECUTE(Post, POST);
	CONNECTED_EXECUTE(Put, PUT);
	CONNECTED_EXECUTE(Delete, DELETE);
	CONNECTED_EXECUTE(Trace, TRACE);
	CONNECTED_EXECUTE(Connect, CONNECT);

	CONNECTED_EXECUTE_ASYNC(Options, OPTIONS);
	CONNECTED_EXECUTE_ASYNC(Get, GET);
	CONNECTED_EXECUTE_ASYNC(Head, HEAD);
	CONNECTED_EXECUTE_ASYNC(Post, POST);
	CONNECTED_EXECUTE_ASYNC(Put, PUT);
	CONNECTED_EXECUTE_ASYNC(Delete, DELETE);
	CONNECTED_EXECUTE_ASYNC(Trace, TRACE);
	CONNECTED_EXECUTE_ASYNC(Connect, CONNECT);

	bool HttpSocket::_executeMethodInternal(HttpResponse* response, chstr method, Url& url, hmap<hstr, hstr>& customHeaders)
	{
		if (response == NULL)
		{
			hlog::warn(sakit::logTag, "Cannot execute, response is NULL!");
			return false;
		}
		if (!url.isValid())
		{
			hlog::warn(sakit::logTag, "Cannot execute, URL is not valid!");
			return false;
		}
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		if (!this->_checkExecuteStatus(state))
		{
			return false;
		}
		hstr request = this->_processRequest(method, url, customHeaders);
		bool result = this->socket->connect(this->host, (this->url.getPort() == 0 ? this->port : this->url.getPort()));
		if (!result)
		{
			this->_terminateConnection();
			return false;
		}
		if (SocketBase::_send(request) == 0)
		{
			this->_terminateConnection();
			return false;
		}
		response->clear();
		if (this->_receiveHttpDirect(response) == 0)
		{
			this->_terminateConnection();
			return false;
		}
		response->Body.rewind();
		response->Raw.rewind();
		if (!this->keepAlive || response->Headers.try_get_by_key("Connection", "") == "close")
		{
			this->_terminateConnection();
		}
		return true;
	}

	bool HttpSocket::_executeMethod(HttpResponse* response, chstr method, Url& url, hmap<hstr, hstr>& customHeaders)
	{
		if (this->isConnected())
		{
			hlog::warn(sakit::logTag, "Already existing connection will be closed!");
			this->_terminateConnection();
		}
		return this->_executeMethodInternal(response, method, url, customHeaders);
	}

	bool HttpSocket::_executeMethod(HttpResponse* response, chstr method, hmap<hstr, hstr>& customHeaders)
	{
		if (!this->isConnected())
		{
			hlog::warn(sakit::logTag, "Cannot execute, there is no existing connection!");
			return false;
		}
		return this->_executeMethodInternal(response, method, this->url, customHeaders);
	}

	bool HttpSocket::_executeMethodInternalAsync(chstr method, Url& url, hmap<hstr, hstr>& customHeaders)
	{
		if (!url.isValid())
		{
			hlog::warn(sakit::logTag, "Cannot execute, URL is not valid!");
			return false;
		}
		this->thread->mutex.lock();
		State state = this->thread->state;
		if (!this->_checkExecuteStatus(state))
		{
			this->thread->mutex.unlock();
			return false;
		}
		hstr request = this->_processRequest(method, url, customHeaders);
		this->thread->response->clear();
		this->thread->stream->clear();
		this->thread->stream->write_raw((void*)request.c_str(), request.size());
		this->thread->stream->rewind();
		this->thread->host = this->host;
		this->thread->port = (this->url.getPort() == 0 ? this->port : this->url.getPort());
		this->thread->state = RUNNING;
		this->thread->mutex.unlock();
		this->thread->start();
		return true;
	}

	bool HttpSocket::_executeMethodAsync(chstr method, Url& url, hmap<hstr, hstr>& customHeaders)
	{
		if (this->isConnected())
		{
			hlog::warn(sakit::logTag, "Already existing connection will be closed!");
			this->_terminateConnection();
		}
		return this->_executeMethodInternalAsync(method, url, customHeaders);
	}

	bool HttpSocket::_executeMethodAsync(chstr method, hmap<hstr, hstr>& customHeaders)
	{
		if (!this->isConnected())
		{
			hlog::warn(sakit::logTag, "Cannot execute, there is no existing connection!");
			return false;
		}
		return this->_executeMethodInternalAsync(method, this->url, customHeaders);
	}

	int HttpSocket::_receiveHttpDirect(HttpResponse* response)
	{
		hmutex mutex;
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
		int retryAttempts = sakit::getRetryAttempts();
		int size = 0;
		int lastSize = 0;
		while (retryAttempts > 0)
		{
			if (!this->socket->receive(response, mutex))
			{
				break;
			}
			if (response->HeadersComplete && response->BodyComplete)
			{
				break;
			}
			size = response->Raw.size();
			if (lastSize != size)
			{
				lastSize = size;
				// retry attempts are reset after a successful read
				retryAttempts = sakit::getRetryAttempts();
				continue;
			}
			retryAttempts--;
			hthread::sleep(retryTimeout);
		}
		// if timed out, has no predefined length, all headers were received and there is a body
		if (retryAttempts == 0 && !response->Headers.has_key("Content-Length") && response->HeadersComplete && response->Body.size() > 0)
		{
			// let's say it's complete, we don't know its supposed length anyway
			response->BodyComplete = true;
		}
		return response->Raw.size();
	}

	int HttpSocket::_send(hstream* stream, int count)
	{
		return this->_sendDirect(stream, count);
	}

	bool HttpSocket::_sendAsync(hstream* stream, int count)
	{
		// not used
		return false;
	}

	void HttpSocket::_terminateConnection()
	{
		this->socket->disconnect();
		this->url = Url();
	}

	bool HttpSocket::_checkExecuteStatus(State senderState)
	{
		if (senderState == RUNNING)
		{
			hlog::warn(sakit::logTag, "Cannot execute, already executing!");
			return false;
		}
		return true;
	}

	hstr HttpSocket::_processRequest(chstr method, Url url, hmap<hstr, hstr> customHeaders)
	{
		this->url = url;
		this->host = Host(this->url.getHost());
		customHeaders["Host"] = this->host.toString();
		customHeaders["Connection"] = (this->keepAlive ? "keep-alive" : "close");
		if (!customHeaders.has_key("Accept-Encoding"))
		{
			customHeaders["Accept-Encoding"] = "identity";
		}
		if (!customHeaders.has_key("Content-Type"))
		{
			customHeaders["Content-Type"] = "application/x-www-form-urlencoded";
		}
		if (!customHeaders.has_key("Accept"))
		{
			customHeaders["Accept"] = "*/*";
		}
		bool urlEncoded = (method == SAKIT_HTTP_REQUEST_GET || method == SAKIT_HTTP_REQUEST_HEAD || method == SAKIT_HTTP_REQUEST_OPTIONS);
		hstr absolutePath;
		hstr body;
		if (!urlEncoded)
		{
			absolutePath = this->url.getAbsolutePath();
			body = this->url.getBody();
			if (body != "")
			{
				customHeaders["Content-Length"] = hstr(body.size());
			}
		}
		else
		{
			absolutePath = this->url.toString(false);
		}
		hstr request;
		request += method + " " + absolutePath + " " + this->_makeProtocol() + SAKIT_HTTP_LINE_ENDING;
		foreach_m (hstr, it, customHeaders)
		{
			request += it->first + ": " + it->second + SAKIT_HTTP_LINE_ENDING;
		}
		request += SAKIT_HTTP_LINE_ENDING;
		if (body != "")
		{
			request += body + SAKIT_HTTP_LINE_ENDING;
		}
		return request;
	}

	hstr HttpSocket::_makeProtocol()
	{
		switch (this->protocol)
		{
		//case HTTP10:	return "HTTP/1.0";
		case HTTP11:	return "HTTP/1.1";
		//case HTTP20:	return "HTTP/2.0";
		}
		hlog::error(sakit::logTag, "Invalid HTTP protocol version!");
		return ""; // invalid
	}

}
