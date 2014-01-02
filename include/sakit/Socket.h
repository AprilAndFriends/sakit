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

#include "Base.h"
#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class ReceiverThread;
	class SenderThread;
	class SocketDelegate;

	class sakitExport Socket : public Base
	{
	public:
		Socket(SocketDelegate* socketDelegate);
		~Socket();

		bool isConnected();

		// TODOsock - make it work with chstr port as well
		bool connect(Ip host, unsigned short port);
		bool disconnect();
		// TODOsock - refactor to work more like write_raw with "count" parameter
		void send(hsbase* stream, int maxBytes = INT_MAX);
		//void send(chstr data); // TODOsock
		void receive(int maxBytes = INT_MAX);

		void update(float timeSinceLastFrame);

	protected:
		SocketDelegate* socketDelegate;
		SenderThread* sender;
		ReceiverThread* receiver;

		void _updateSending();
		void _updateReceiving();

		static void _receive(SocketDelegate* socketDelegate);

	};

}
#endif
