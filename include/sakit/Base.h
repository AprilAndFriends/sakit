/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a base class for all socket operations.

#ifndef SAKIT_BASE_H
#define SAKIT_BASE_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;

	class sakitExport Base
	{
	public:
		friend class PlatformSocket;

		Base();
		virtual ~Base();

		HL_DEFINE_GET(Ip, host, Host);
		HL_DEFINE_GET(unsigned short, port, Port);
		hstr getFullHost();

		virtual void update(float timeSinceLastFrame) = 0;

	protected:
		PlatformSocket* socket;
		Ip host;
		unsigned short port;

	};

}
#endif
