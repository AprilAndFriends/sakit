/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"

namespace sakit
{
	Socket::Socket() : host("")
	{
		this->socket = new PlatformSocket();
	}

	Socket::~Socket()
	{
		delete this->socket;
	}

	hstr Socket::getFullHost()
	{
		return (this->isConnected() ? hsprintf("%s:%d", this->host.getAddress().c_str(), this->port) : "");
	}

	bool Socket::isConnected()
	{
		return this->socket->isConnected();
	}

	bool Socket::connect(Ip host, unsigned short port)
	{
		if (this->disconnect()) // disconnect first
		{
			hlog::warn(sakit::logTag, "Connection already existed, it was closed.");
		}
		bool result = this->socket->connect(host, port);
		if (result)
		{
			this->host = host;
			this->port = port;
		}
		return result;
	}
	
	bool Socket::disconnect()
	{
		this->host = Ip("");
		this->port = 0;
		return this->socket->disconnect();
	}

	long Socket::receive(hsbase& stream, bool retainPosition)
	{
		return this->receive(stream, INT_MAX, retainPosition);
	}
	
	long Socket::receive(hsbase& stream, int maxBytes, bool retainPosition)
	{
		if (!this->isConnected())
		{
			hlog::error(sakit::logTag, "Not connected!");
			return 0;
		}
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot read: maxBytes is 0!");
			return 0;
		}
		hstream data;
		this->socket->receive(data, maxBytes);
		int size = data.size();
		if (size > 0)
		{
			data.rewind();
			stream.write_raw(data);
			if (retainPosition)
			{
				stream.seek(size);
			}
		}
		// TODOsock - call delegate here?
		return size;
	}
	
}
