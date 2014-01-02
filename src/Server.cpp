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
#include "Server.h"

namespace sakit
{
	Server::Server(ServerDelegate* serverDelegate) : Base()
	{
		this->serverDelegate = serverDelegate;
	}

	Server::~Server()
	{
	}

	bool Server::isBound()
	{
		return this->socket->isConnected();
	}

	bool Server::bind(Ip host, unsigned short port)
	{
		if (this->unbind()) // unbind first
		{
			hlog::warn(sakit::logTag, "Server already bound, it was unbound.");
		}
		bool result = this->socket->bind(host, port);
		if (result)
		{
			this->host = host;
			this->port = port;
		}
		return result;
	}

	bool Server::unbind()
	{
		this->host = Ip("");
		this->port = 0;
		return this->socket->disconnect();
	}
	
}
