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

#include "sakitExport.h"
#include "SocketBase.h"
#include "State.h"

namespace sakit
{
	class ReceiverThread;
	class SenderThread;
	class SocketDelegate;

	class sakitExport Socket : public SocketBase
	{
	public:
		~Socket();

		bool isSending();
		bool isReceiving();

		void update(float timeSinceLastFrame);

		int send(hstream* stream, int count = INT_MAX);
		int send(chstr data);

		bool sendAsync(hstream* stream, int count = INT_MAX);
		bool sendAsync(chstr data);
		bool stopReceiveAsync();

	protected:
		SocketDelegate* socketDelegate;
		SenderThread* sender;
		ReceiverThread* receiver;
		State idleState;

		Socket(SocketDelegate* socketDelegate, State idleState);

		int _send(hstream* stream, int count);
		bool _prepareReceive(hstream* stream);
		int _finishReceive(int result);
		bool _startReceiveAsync(int maxValue);

		void _updateSending();
		virtual void _updateReceiving() = 0;

		bool _checkStartReceiveStatus(State receiverState);

		bool _canSend(State state);
		bool _canReceive(State state);
		bool _canStopReceive(State state);
		bool _checkSendParameters(hstream* stream, int count);
		bool _checkReceiveParameters(hstream* stream);

	};

}
#endif
