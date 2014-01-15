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

#define HTTP_LINE_ENDING "\r\n"
#define HTTP_REQUEST_GET "GET"
#define HTTP_REQUEST_POST "POST"

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

	bool HttpSocket::executeGet(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders)
	{
		return this->_executeMethod(response, HTTP_REQUEST_GET, url, customHeaders);
	}

	bool HttpSocket::executePost(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders)
	{
		return this->_executeMethod(response, HTTP_REQUEST_POST, url, customHeaders);
	}

	bool HttpSocket::_executeMethod(HttpResponse* response, chstr method, Url url, hmap<hstr, hstr>& customHeaders)
	{
		if (response == NULL)
		{
			hlog::warn(sakit::logTag, "Cannot execute, response is NULL!");
			return false;
		}
		hstr request = this->_makeRequest(method, url, customHeaders);
		if (!this->socket->connect(this->host, this->port))
		{
			return false;
		}
		if (SocketBase::_send(request) == 0)
		{
			return false;
		}
		response->Raw.clear();
		if (this->_receiveDirect(&response->Raw, INT_MAX) == 0)
		{
			return false;
		}
		response->Raw.rewind();
		if (!this->keepAlive)
		{
			this->socket->disconnect();
		}
		return response->parseFromRaw();
	}

	int HttpSocket::_send(hstream* stream, int count)
	{
		return this->_sendDirect(stream, count);
	}

	bool HttpSocket::_sendAsync(hstream* stream, int count)
	{
		return false;
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
		request += method + " " + url.getAbsolutePath() + " " + this->_makeProtocol() + HTTP_LINE_ENDING;
		foreach_m (hstr, it, customHeaders)
		{
			request += it->first + ": " + it->second + HTTP_LINE_ENDING;
		}
		if (body.size() > 0)
		{
			request += HTTP_LINE_ENDING + body + HTTP_LINE_ENDING;
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
