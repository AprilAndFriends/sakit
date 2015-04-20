/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "HttpResponse.h"
#include "HttpSocket.h"
#include "HttpSocketDelegate.h"
#include "HttpSocketThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "sakitUtil.h"
#include "State.h"

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

	HttpSocket::HttpSocket(HttpSocketDelegate* socketDelegate, Protocol protocol) : SocketBase(), keepAlive(false), forceUrlEncoding(false)
	{
		this->socketDelegate = socketDelegate;
		this->protocol = protocol;
		this->remotePort = HttpSocket::DefaultPort;
		this->socket->setConnectionLess(false);
		this->thread = new HttpSocketThread(this->socket, &this->timeout, &this->retryFrequency);
		this->__register();
	}

	HttpSocket::~HttpSocket()
	{
		this->__unregister();
		this->thread->join();
		delete this->thread;
	}

	bool HttpSocket::isConnected()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == RUNNING || this->state == CONNECTED);
	}

	bool HttpSocket::isExecuting()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == RUNNING);
	}

	void HttpSocket::update(float timeDelta)
	{
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->thread->mutex);
		State result = this->thread->result;
		if (result == RUNNING || result == IDLE)
		{
			return;
		}
		this->thread->result = IDLE;
		HttpResponse* response = this->thread->response;
		this->thread->response = new HttpResponse();
		if (!this->keepAlive || response->Headers.tryGet("Connection", "") == "close" || !this->socket->isConnected())
		{
			this->_terminateConnection();
			this->state = IDLE;
		}
		else
		{
			this->state = CONNECTED;
		}
		lockThread.release();
		lock.release();
		switch (result)
		{
		case FINISHED:	this->socketDelegate->onExecuteCompleted(this, response, this->url);	break;
		case FAILED:	this->socketDelegate->onExecuteFailed(this, response, this->url);		break;
		}
		delete response;
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
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canExecute(this->state))
		{
			return false;
		}
		this->state = RUNNING;
		lock.release();
		hstr request = this->_processRequest(method, url, customHeaders);
		unsigned short port = (this->url.getPort() == 0 ? this->remotePort : this->url.getPort());
		bool result = this->socket->connect(this->remoteHost, port, this->localHost, this->localPort, this->timeout, this->retryFrequency);
		if (!result)
		{
			this->_terminateConnection();
			lock.acquire(&this->mutexState);
			this->state = IDLE;
			return false;
		}
		if (SocketBase::_send(request) == 0)
		{
			this->_terminateConnection();
			lock.acquire(&this->mutexState);
			this->state = IDLE;
			return false;
		}
		response->clear();
		if (this->_receiveHttpDirect(response) == 0)
		{
			this->_terminateConnection();
			lock.acquire(&this->mutexState);
			this->state = IDLE;
			return false;
		}
		response->Body.rewind();
		response->Raw.rewind();
		if (!this->keepAlive || response->Headers.tryGet("Connection", "") == "close")
		{
			this->_terminateConnection();
			lock.acquire(&this->mutexState);
			this->state = IDLE;
		}
		else
		{
			lock.acquire(&this->mutexState);
			this->state = CONNECTED;
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
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->thread->mutex);
		if (!this->_canExecute(this->state))
		{
			return false;
		}
		hstr request = this->_processRequest(method, url, customHeaders);
		this->thread->response->clear();
		this->thread->stream->clear();
		this->thread->stream->writeRaw((void*)request.cStr(), request.size());
		this->thread->stream->rewind();
		this->thread->host = this->remoteHost;
		this->thread->port = (this->url.getPort() == 0 ? this->remotePort : this->url.getPort());
		this->state = RUNNING;
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
		float time = 0.0f;
		int64_t size = 0;
		int64_t lastSize = 0;
		while (true)
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
				time = 0.0f;
				continue;
			}
			time += this->retryFrequency;
			if (time >= this->timeout)
			{
				break;
			}
			hthread::sleep(this->retryFrequency * 1000.0f);
		}
		// if timed out, has no predefined length, all headers were received and there is a body
		if (time >= this->timeout && !response->Headers.hasKey("Content-Length") && response->HeadersComplete && response->Body.size() > 0)
		{
			// let's say it's complete, we don't know its supposed length anyway
			response->BodyComplete = true;
		}
		return (int)response->Raw.size();
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

	bool HttpSocket::_canExecute(State state)
	{
		harray<State> allowed;
		allowed += IDLE;
		allowed += CONNECTED;
		return _checkState(state, allowed, "execute");
	}

	hstr HttpSocket::_processRequest(chstr method, Url url, hmap<hstr, hstr> customHeaders)
	{
		this->url = url;
		this->remoteHost = Host(this->url.getHost());
		customHeaders["Host"] = this->remoteHost.toString();
		customHeaders["Connection"] = (this->keepAlive ? "keep-alive" : "close");
		if (!customHeaders.hasKey("Accept-Encoding"))
		{
			customHeaders["Accept-Encoding"] = "identity";
		}
		if (!customHeaders.hasKey("Content-Type"))
		{
			customHeaders["Content-Type"] = "application/x-www-form-urlencoded";
		}
		if (!customHeaders.hasKey("Accept"))
		{
			customHeaders["Accept"] = "*/*";
		}
		bool urlEncoded = (this->forceUrlEncoding || method == SAKIT_HTTP_REQUEST_GET || method == SAKIT_HTTP_REQUEST_HEAD || method == SAKIT_HTTP_REQUEST_OPTIONS);
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
