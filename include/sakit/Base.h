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
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;

	class sakitExport Base
	{
	public:
		friend class PlatformSocket;

		virtual ~Base();

		HL_DEFINE_GET(Host, host, Host);
		HL_DEFINE_GET(unsigned short, port, Port);
		hstr getFullHost();

		virtual void update(float timeSinceLastFrame) = 0;

	protected:
		PlatformSocket* socket;
		Host host;
		unsigned short port;

		Base();

		int _sendDirect(hstream* stream, int count);
		int _receiveDirect(hstream* stream, int maxBytes);
		int _receiveFromDirect(hstream* stream, Host& host, unsigned short& port);

		bool _canSend(hstream* stream, int count);
		bool _canReceive(hstream* stream);

		virtual void _activateConnection(Host host, unsigned short port);

		void __register();
		void __unregister();

	};

}
#endif
