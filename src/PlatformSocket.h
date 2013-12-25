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
/// Defines a platform dependent implementation for socket functionality.

#ifndef SAKIT_PLATFORM_SOCKET_H
#define SAKIT_PLATFORM_SOCKET_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class sakitExport PlatformSocket
	{
	public:
		PlatformSocket();
		~PlatformSocket();

		HL_DEFINE_IS(connected, Connected);

		bool connect(chstr host, unsigned int port);
		bool disconnect();

		static void platformInit();
		static void platformDestroy();

	protected:
		bool connected;
#if !defined(_WIN32) || !defined(_WINRT)
		unsigned int sock;
		struct addrinfo* info;
#else
		Windows::Networking::Sockets::StreamSocket^ sock;
		Windows::Networking::HostName^ hostName;
		bool _asyncProcessing;
		bool _asyncFinished;
		unsigned int _asyncSize;
		bool _awaitAsync();
#endif

	};

}
#endif
