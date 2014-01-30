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
/// Defines a base class for all basic socket operations.

#ifndef SAKIT_SOCKET_BASE_H
#define SAKIT_SOCKET_BASE_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Base.h"
#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;
	class ReceiverThread;
	class SenderThread;

	class sakitExport SocketBase : public Base
	{
	public:
		friend class PlatformSocket;

		~SocketBase();

		HL_DEFINE_GET(Host, remoteHost, RemoteHost);
		HL_DEFINE_GET(unsigned short, remotePort, RemotePort);

	protected:
		SocketBase();

		Host remoteHost;
		unsigned short remotePort;

		virtual int _send(hstream* stream, int count) = 0;
		virtual int _send(chstr data);

		virtual void _activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort);

	};

}
#endif
