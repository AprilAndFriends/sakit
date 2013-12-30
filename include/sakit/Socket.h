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
#include <hltypes/hmutex.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;
	class SocketDelegate;
	class ReceiverThread;

	class sakitExport Socket
	{
	public:
		Socket(SocketDelegate* socketDelegate);
		virtual ~Socket();

		HL_DEFINE_GET(Ip, host, Host);
		HL_DEFINE_GET(unsigned short, port, Port);
		hstr getFullHost();
		bool isConnected();

		// TODOsock - make it work with chstr port as well
		bool connect(Ip host, unsigned short port);
		bool disconnect();
		void receive(int maxBytes = INT_MAX);
		void send(hsbase* stream);
		//void send(chstr data); // TODOsock

		void update(float timeSinceLastFrame);

	protected:
		PlatformSocket* socket;
		SocketDelegate* socketDelegate;
		ReceiverThread* receiver;
		Ip host;
		unsigned short port;

		static void _receive(SocketDelegate* socketDelegate);

	};

}
#endif
