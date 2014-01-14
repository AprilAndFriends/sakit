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

#include "sakit.h"
#include "HttpSocket.h"

#define HTTP_LINE_ENDING "\r\n"

namespace sakit
{
	HttpSocket::HttpSocket(SocketDelegate* socketDelegate) : TcpSocket(socketDelegate), port(80), protocol(HTTP11)
	{
	}

	HttpSocket::~HttpSocket()
	{
	}
	
	bool HttpSocket::connect(Host host)
	{
		return TcpSocket::connect(host, this->port);
	}
	
	bool HttpSocket::connectAsync(Host host)
	{
		return TcpSocket::connectAsync(host, this->port);
	}
	
	int HttpSocket::get(chstr url)
	{
		hstr data;
		data += "GET " + this->_makeUrl(url) + " " + this->_makeProtocol() + HTTP_LINE_ENDING;
		data += "Host: " + this->host.getAddress() + HTTP_LINE_ENDING;
		return this->send(data);
	}

	int HttpSocket::post(chstr url, hmap<hstr, hstr> parameters)
	{
		harray<hstr> params;
		foreach_m (hstr, it, parameters)
		{
			params += it->first + "=" + it->second;
		}
		hstr postData = params.join('&');
		hstr data;
		data += "POST " + this->_makeUrl(url) + " " + this->_makeProtocol() + HTTP_LINE_ENDING;
		data += "Host: " + this->host.getAddress() + HTTP_LINE_ENDING;
		data += "Content-Length: " + hstr(postData.size()) + HTTP_LINE_ENDING;
		data += this->_makeHeaders();
		data += HTTP_LINE_ENDING;
		data += postData;
		data += HTTP_LINE_ENDING;
		return this->send(data);
	}

	hstr HttpSocket::_makeUrl(chstr url)
	{
		hstr address = this->host.getAddress();
		if (url.contains(address))
		{
			address = url;
		}
		else
		{
			address = address.rtrim('/') + "/" + url.ltrim('/');
		}
		if (!address.starts_with("http://"))
		{
			address = "http://" + address;
		}
		return address;
	}

	hstr HttpSocket::_makeProtocol()
	{
		switch (this->protocol)
		{
		case HTTP10:	return "HTTP/1.0";
		case HTTP11:	return "HTTP/1.1";
		case HTTP20:	return "HTTP/2.0";
		}
		hlog::error(sakit::logTag, "Invalid HTTP protocol version!");
		return ""; // invalid
	}

	hstr HttpSocket::_makeHeaders()
	{
		harray<hstr> results;
		foreach_m (hstr, it, this->headers)
		{
			if (it->first != "Host" && it->first != "Content-Length")
			{
				results += hsprintf("%s: %s%s", it->first.c_str(), it->second.c_str(), HTTP_LINE_ENDING);
			}
		}
		return results.join("");
	}

}
