/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"

namespace sakit
{
	Socket::Socket()
	{
		this->socket = new PlatformSocket();
	}

	Socket::~Socket()
	{
		delete this->socket;
	}

	bool Socket::isConnected()
	{
		return this->socket->isConnected();
	}

	bool Socket::connect(chstr host, unsigned short port)
	{
		if (this->disconnect()) // disconnect first
		{
			hlog::warn(sakit::logTag, "Connection already existed, it was closed.");
		}
		return this->socket->connect(host, port);
	}
	
	bool Socket::disconnect()
	{
		return this->socket->disconnect();
	}
	
}
