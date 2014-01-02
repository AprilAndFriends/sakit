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
	hstr logTag = "sakit";
	float retryTimeout = 0.1f;
	int bufferSize = 65536;
	harray<Socket*> sockets;

	void init(int bufferSize)
	{
		hlog::write(sakit::logTag, "Initializing Socket Abstraction Kit.");
		PlatformSocket::platformInit();
		sakit::bufferSize = bufferSize;
	}

	void destroy()
	{
		hlog::write(sakit::logTag, "Destroying Socket Abstraction Kit.");
		PlatformSocket::platformDestroy();
		if (sockets.size() > 0)
		{
			hlog::warn(sakit::logTag, "Not all sockets have been destroyed!");
		}
	}

	void update(float timeSinceLastFrame)
	{
		foreach (Socket*, it, sockets)
		{
			(*it)->update(timeSinceLastFrame);
		}
	}

	float getRetryTimeout()
	{
		return retryTimeout;
	}

	void setRetryTimeout(float value)
	{
		retryTimeout = value;
	}

}
