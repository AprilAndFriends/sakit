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
	class SocketThread;

	class sakitExport Socket : public Base
	{
	public:
		enum State
		{
			IDLE,
			CONNECTING,
			CONNECTED,
			RUNNING,
			DISCONNECTING
		};

		Socket(SocketDelegate* socketDelegate);
		~Socket();

		bool isConnecting();
		bool isConnected();
		bool isSending();
		bool isReceiving();
		bool isDisconnecting();

		// TODOsock - make it work with chstr port as well
		bool connect(Ip host, unsigned short port);
		bool disconnect();
		// TODOsock - refactor to work more like write_raw with "count" parameter
		int send(hsbase* stream, int maxBytes = INT_MAX);
		//int send(chstr data); // TODOsock
		hsbase* receive(int maxBytes = INT_MAX);

		// TODOsock - make it work with chstr port as well
		bool connectAsync(Ip host, unsigned short port);
		bool disconnectAsync();
		// TODOsock - refactor to work more like write_raw with "count" parameter
		bool sendAsync(hsbase* stream, int maxBytes = INT_MAX);
		//bool sendAsync(chstr data); // TODOsock
		bool receiveAsync(int maxBytes = INT_MAX);

		void update(float timeSinceLastFrame);

	protected:
		SocketDelegate* socketDelegate;
		SocketThread* thread;
		SenderThread* sender;
		ReceiverThread* receiver;

		void _activateConnection(Ip host, unsigned short port);

		void _updateSending();
		void _updateReceiving();

		bool _checkConnectStatus(State socketState);
		bool _checkSendStatus(State socketState, State senderState);
		bool _checkReceiveStatus(State socketState, State receiverState);
		bool _checkDisconnectStatus(State socketState, State senderState, State receiverState);

	};

}
#endif
