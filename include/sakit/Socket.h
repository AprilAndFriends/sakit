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
/// Defines a basic socket class.

#ifndef SAKIT_SOCKET_H
#define SAKIT_SOCKET_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;

	class sakitExport Socket
	{
	public:
		Socket();
		virtual ~Socket();

		bool isConnected();

		bool connect(chstr host, unsigned short port);
		bool disconnect();
		long receive(hsbase& stream, int maxBytes, bool retainPosition = true);
		long receive(hsbase& stream, bool retainPosition = true);

	protected:
		PlatformSocket* socket;

	};

}
#endif
