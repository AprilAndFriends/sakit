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
#include "HttpTcpSocketDelegate.h"
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
	}

	HttpSocket::~HttpSocket()
	{
	}

	bool HttpSocket::isConnected()
	{
		return this->url.isValid();
	}

	bool HttpSocket::isExecuting()
	{
		// TODOsock - implement
		return false;
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
		unsigned short currentPort = this->port;
		hstr host = url.getHost();
		int index = host.find(':');
		if (index >= 0)
		{
			currentPort = (unsigned short)(int)host(index + 1, -1);
		}
		hstr request = this->_makeRequest(method, url, customHeaders);
		bool result = this->socket->connect(this->host, currentPort);
		if (!result)
		{
			return false;
		}
		this->url = url;
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
		HttpResponse response;
		// TODOsock - implement real async
		bool result = this->_executeMethodInternal(&response, method, url, customHeaders);
		if (result)
		{
			this->socketDelegate->onExecuteCompleted(this, &response, url);
		}
		else
		{
			this->socketDelegate->onExecuteFailed(this, &response, url);
		}
		return result;
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
		// TODOsock - implement
		return false;
	}

	void HttpSocket::_terminateConnection()
	{
		this->socket->disconnect();
		this->url = Url();
	}

	void HttpSocket::_updateSending()
	{
		// TODOsock - implement
	}

	void HttpSocket::_updateReceiving()
	{
		// TODOsock - implement
	}

	hstr HttpSocket::_makeRequest(chstr method, Url url, hmap<hstr, hstr> customHeaders)
	{
		this->host = Host(url.getHost());
		hstr body = url.getBody();
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
		if (body.size() > 0)
		{
			customHeaders["Content-Length"] = hstr(body.size());
		}
		hstr request;
		request += method + " " + url.getAbsolutePath() + " " + this->_makeProtocol() + SAKIT_HTTP_LINE_ENDING;
		foreach_m (hstr, it, customHeaders)
		{
			request += it->first + ": " + it->second + SAKIT_HTTP_LINE_ENDING;
		}
		if (body.size() > 0)
		{
			request += SAKIT_HTTP_LINE_ENDING + body + SAKIT_HTTP_LINE_ENDING;
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
