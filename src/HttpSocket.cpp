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

	bool HttpSocket::isConnected()
	{
		return this->url.isValid();
	}

	bool HttpSocket::executeGet(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders)
	{
		return this->_executeMethod(response, HTTP_REQUEST_GET, url, customHeaders);
	}

	bool HttpSocket::executeGet(HttpResponse* response, hmap<hstr, hstr> customHeaders)
	{
		return this->_executeMethod(response, HTTP_REQUEST_GET, customHeaders);
	}

	bool HttpSocket::executePost(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders)
	{
		return this->_executeMethod(response, HTTP_REQUEST_POST, url, customHeaders);
	}

	bool HttpSocket::executePost(HttpResponse* response, hmap<hstr, hstr> customHeaders)
	{
		return this->_executeMethod(response, HTTP_REQUEST_POST, customHeaders);
	}

	bool HttpSocket::_executeMethodInternal(HttpResponse* response, chstr method, Url& url, hmap<hstr, hstr>& customHeaders)
	{
		if (response == NULL)
		{
			hlog::warn(sakit::logTag, "Cannot execute, response is NULL!");
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
		response->Raw.clear();
		// TODOsock - should parse HTTP data as it is received
		if (this->_receiveDirect(&response->Raw, INT_MAX) == 0)
		{
			this->_terminateConnection();
			return false;
		}
		response->Raw.rewind();
		result = response->parseFromRaw();
		if (result)
		{
			/*if (response->Headers.has_key("Content-Length"))
			{
				// there is an exact value of how big the body is
				int remaining = (int)response->Headers["Content-Length"] - response->Body.size();
				if (remaining > 0)
				{
					response->Raw.seek(0, hstream::END);
					int received = 0;
					while (remaining > 0)
					{
						received = this->_receiveDirect(&response->Raw, INT_MAX);
						if (received == 0)
						{
							break;
						}
						remaining -= received;
					}
					result &= response->parseFromRaw();
				}
			}
			else*/ if (response->Headers.try_get_by_key("Transfer-Encoding", "") == "chunked")
			{
				harray<hstr> data = response->Body.split('\n', 1);
				if (data.size() == 2)
				{
					// there is an exact value of how big the body is going to be
					int remaining = data[0].replace("\r", "").unhex() - data[1].size();
					if (remaining > 0)
					{
						response->Raw.seek(0, hstream::END);
						int received = 0;
						while (remaining > 0)
						{
							received = this->_receiveDirect(&response->Raw, INT_MAX);
							if (received == 0)
							{
								break;
							}
							remaining -= received;
						}
						result &= response->parseFromRaw();
					}
				}
			}
			else
			{
				// TODOsock - keep receiving until either timeout or all data has been received
			}
		}
		if (!this->keepAlive || response->Headers.try_get_by_key("Connection", "") == "close")
		{
			this->_terminateConnection();
		}
		return result;
	}

	bool HttpSocket::_executeMethod(HttpResponse* response, chstr method, Url& url, hmap<hstr, hstr>& customHeaders)
	{
		if (this->isConnected())
		{
			hlog::warn(sakit::logTag, "Already existing connection will be closed!");
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
