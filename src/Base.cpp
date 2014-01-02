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

#include "Base.h"
#include "PlatformSocket.h"

namespace sakit
{
	Base::Base() : host(""), port(0)
	{
		this->socket = new PlatformSocket();
	}

	Base::~Base()
	{
		delete this->socket;
	}

	hstr Base::getFullHost()
	{
		return (this->socket->isConnected() ? hsprintf("%s:%d", this->host.getAddress().c_str(), this->port) : "");
	}

}
