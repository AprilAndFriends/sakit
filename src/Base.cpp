/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>

#include "Base.h"
#include "PlatformSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;

	Base::Base() : host(""), port(0)
	{
		connectionsMutex.lock();
		connections += this;
		connectionsMutex.unlock();
		this->socket = new PlatformSocket();
	}

	Base::~Base()
	{
		delete this->socket;
		connectionsMutex.lock();
		int index = connections.index_of(this);
		if (index >= 0)
		{
			connections.remove_at(index);
		}
		connectionsMutex.unlock();
	}

	hstr Base::getFullHost()
	{
		return (this->socket->isConnected() ? hsprintf("%s:%d", this->host.getAddress().c_str(), this->port) : "");
	}

	void Base::_activateConnection(Ip host, unsigned short port)
	{
		this->host = host;
		this->port = port;
	}

}
