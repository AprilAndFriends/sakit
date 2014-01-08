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
#include <hltypes/hstream.h>
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

		~Socket();

		bool isSending();
		bool isReceiving();

		void update(float timeSinceLastFrame);

		virtual int send(hstream* stream, int count = INT_MAX) = 0;
		virtual int send(chstr data);
		virtual bool sendAsync(hstream* stream, int count = INT_MAX) = 0;
		virtual bool sendAsync(chstr data);

	protected:
		SocketDelegate* socketDelegate;
		SenderThread* sender;
		ReceiverThread* receiver;

		Socket(SocketDelegate* socketDelegate);

		bool _checkSendStatus(State senderState);
		bool _checkReceiveStatus(State receiverState);

	};

}
#endif
