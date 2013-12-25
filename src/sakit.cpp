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

#include "sakit.h"
#include "PlatformSocket.h"

namespace sakit
{
	hstr logTag = "sakit";

	void init()
	{
		hlog::write(sakit::logTag, "Initializing Socket Abstraction Kit.");
		PlatformSocket::platformInit();
	}

	void destroy()
	{
		hlog::write(sakit::logTag, "Destroying Socket Abstraction Kit.");
		PlatformSocket::platformDestroy();
	}

}
