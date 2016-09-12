/// @file
/// @version 1.05
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

#define HTTP_DELIMITER "\r\n"
#define REQUEST_OPTIONS "OPTIONS"
#define REQUEST_GET "GET"
#define REQUEST_HEAD "HEAD"
#define REQUEST_POST "POST"
#define REQUEST_PUT "PUT"
#define REQUEST_DELETE "DELETE"
#define REQUEST_TRACE "TRACE"
#define REQUEST_CONNECT "CONNECT"

#define NORMAL_EXECUTE(name, constant) \
	bool HttpSocket::execute ## name(HttpResponse* response, Url url, chstr customBody, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethod(response, REQUEST_ ## constant, url, customBody, customHeaders); \
	}
#define NORMAL_EXECUTE_ASYNC(name, constant) \
	bool HttpSocket::execute ## name ## Async(Url url, chstr customBody, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethodAsync(REQUEST_ ## constant, url, customBody, customHeaders); \
	}
#define CONNECTED_EXECUTE(name, constant) \
	bool HttpSocket::execute ## name(HttpResponse* response, chstr customBody, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethod(response, REQUEST_ ## constant, customBody, customHeaders); \
	}
#define CONNECTED_EXECUTE_ASYNC(name, constant) \
	bool HttpSocket::execute ## name ## Async(chstr customBody, hmap<hstr, hstr> customHeaders) \
	{ \
		return this->_executeMethodAsync(REQUEST_ ## constant, customBody, customHeaders); \
	}

namespace sakit
{
	unsigned short HttpSocket::DefaultPort = 80;

	HttpSocket::HttpSocket(HttpSocketDelegate* socketDelegate, Protocol protocol) : SocketBase(), keepAlive(false), reportProgress(false)
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
			if (this->reportProgress && this->thread->response->hasNewData())
			{
				HttpResponse* response = this->thread->response->clone();
				this->thread->response->consumeNewData();
				Url url = this->url;
				lockThread.release();
				lock.release();
				this->socketDelegate->onExecuteProgress(this, response, url);
				delete response;
			}
			return;
		}
		this->thread->result = IDLE;
		HttpResponse* response = this->thread->response;
		this->thread->response = new HttpResponse();
		Url url = this->url; // _terminateConnection() deletes this, but it's needed for the delegate call ahead
		if (!this->keepAlive || response->headers.tryGet(SAKIT_HTTP_REQUEST_HEADER_CONNECTION, "") == "close" || !this->socket->isConnected())
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
		// some final data might be available
		if (this->reportProgress && response->hasNewData())
		{
			this->socketDelegate->onExecuteProgress(this, response, url);
		}
		// the data can be rewinded after reporting progress
		response->raw.rewind();
		response->body.rewind();
		switch (result)
		{
		case FINISHED:	this->socketDelegate->onExecuteCompleted(this, response, url);	break;
		case FAILED:	this->socketDelegate->onExecuteFailed(this, response, url);		break;
		default:																		break;
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

	bool HttpSocket::_executeMethodInternal(HttpResponse* response, chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders)
	{
		if (response == NULL)
		{
			hlog::warn(logTag, "Cannot execute, response is NULL!");
			return false;
		}
		if (!url.isValid())
		{
			hlog::warn(logTag, "Cannot execute, URL is not valid!");
			return false;
		}
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canExecute(this->state))
		{
			return false;
		}
		this->state = RUNNING;
		lock.release();
		hstr request = this->_processRequest(method, url, customBody, customHeaders);
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
		response->body.rewind();
		response->raw.rewind();
		if (!this->keepAlive || response->headers.tryGet(SAKIT_HTTP_REQUEST_HEADER_CONNECTION, "") == "close")
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
		return (response->headersComplete && response->bodyComplete);
	}

	bool HttpSocket::_executeMethod(HttpResponse* response, chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders)
	{
		if (this->isConnected())
		{
			hlog::warn(logTag, "Already existing connection will be closed!");
			this->_terminateConnection();
		}
		return this->_executeMethodInternal(response, method, url, customBody, customHeaders);
	}

	bool HttpSocket::_executeMethod(HttpResponse* response, chstr method, chstr customBody, hmap<hstr, hstr>& customHeaders)
	{
		if (!this->isConnected())
		{
			hlog::warn(logTag, "Cannot execute, there is no existing persistent connection!");
			return false;
		}
		return this->_executeMethodInternal(response, method, this->url, customBody, customHeaders);
	}

	bool HttpSocket::_executeMethodInternalAsync(chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders)
	{
		if (!url.isValid())
		{
			hlog::warn(logTag, "Cannot execute, URL is not valid!");
			return false;
		}
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->thread->mutex);
		if (!this->_canExecute(this->state))
		{
			return false;
		}
		hstr request = this->_processRequest(method, url, customBody, customHeaders);
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

	bool HttpSocket::_executeMethodAsync(chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders)
	{
		if (this->isConnected())
		{
			hlog::warn(logTag, "Already existing connection will be closed!");
			this->_terminateConnection();
		}
		return this->_executeMethodInternalAsync(method, url, customBody, customHeaders);
	}

	bool HttpSocket::_executeMethodAsync(chstr method, chstr customBody, hmap<hstr, hstr>& customHeaders)
	{
		if (!this->isConnected())
		{
			hlog::warn(logTag, "Cannot execute, there is no existing connection!");
			return false;
		}
		return this->_executeMethodInternalAsync(method, this->url, customBody, customHeaders);
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
			if (response->headersComplete && response->bodyComplete)
			{
				break;
			}
			size = response->raw.size();
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
		if (time >= this->timeout && response->headersComplete)
		{
			if (!response->headers.hasKey(SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH) && response->body.size() > 0)
			{
				// let's say it's complete, we don't know its supposed length anyway
				hlog::warn(logTag, "HttpSocket did not return header Content-Length! Body might be incomplete, but will be considered complete.");
				response->bodyComplete = true;
			}
			else if ((int)response->headers[SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH] == 0) // empty body
			{
				response->bodyComplete = true;
			}
		}
		return (int)response->raw.size();
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

	bool HttpSocket::abort()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canAbort(this->state))
		{
			return false;
		}
		this->state = DISCONNECTING;
		lock.release();
		bool result = this->socket->disconnect();
		lock.acquire(&this->mutexState);
		if (result)
		{
			this->remoteHost = Host();
			this->remotePort = 0;
			this->localHost = Host();
			this->localPort = 0;
			this->state = IDLE;
			this->url = Url();
		}
		else
		{
			this->state = state;
		}
		return result;
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

	bool HttpSocket::_canAbort(State state)
	{
		harray<State> allowed;
		allowed += RUNNING;
		return _checkState(state, allowed, "abort");
	}

	hstr HttpSocket::_processRequest(chstr method, Url url, chstr customBody, hmap<hstr, hstr> customHeaders)
	{
		this->url = url;
		this->remoteHost = Host(this->url.getHost());
		customHeaders[SAKIT_HTTP_REQUEST_HEADER_HOST] = this->remoteHost.toString();
		customHeaders[SAKIT_HTTP_REQUEST_HEADER_CONNECTION] = (this->keepAlive ? "keep-alive" : "close");
		if (!customHeaders.hasKey(SAKIT_HTTP_REQUEST_HEADER_ACCEPT_ENCODING))
		{
			customHeaders[SAKIT_HTTP_REQUEST_HEADER_ACCEPT_ENCODING] = "identity";
		}
		if (!customHeaders.hasKey(SAKIT_HTTP_REQUEST_HEADER_CONTENT_TYPE))
		{
			customHeaders[SAKIT_HTTP_REQUEST_HEADER_CONTENT_TYPE] = "application/x-www-form-urlencoded";
		}
		if (!customHeaders.hasKey(SAKIT_HTTP_REQUEST_HEADER_ACCEPT))
		{
			customHeaders[SAKIT_HTTP_REQUEST_HEADER_ACCEPT] = "*/*";
		}
		bool urlEncoded = (customBody == "" && (method == REQUEST_GET || method == REQUEST_HEAD || method == REQUEST_OPTIONS));
		hstr absolutePath;
		hstr body = customBody;
		if (!urlEncoded)
		{
			absolutePath = this->url.getRelativePath();
			if (customBody == "")
			{
				body = this->url.getBody();
			}
			if (body != "")
			{
				customHeaders[SAKIT_HTTP_REQUEST_HEADER_CONTENT_LENGTH] = hstr(body.size());
			}
		}
		else
		{
			absolutePath = this->url.toString(false, true);
		}
		hstr request;
		request += method + " " + absolutePath + " " + this->_makeProtocol() + HTTP_DELIMITER;
		foreach_m (hstr, it, customHeaders)
		{
			request += it->first + ": " + it->second + HTTP_DELIMITER;
		}
		request += HTTP_DELIMITER;
		if (body != "")
		{
			request += body + HTTP_DELIMITER;
		}
		hlog::debug(logTag, "Processed request generated:\n" + request);
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
		hlog::error(logTag, "Invalid HTTP protocol version!");
		return ""; // invalid
	}

}
