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
/// Defines a socket working over TCP.

#ifndef SAKIT_TCP_SOCKET_H
#define SAKIT_TCP_SOCKET_H

#include <hltypes/hstream.h>

#include "sakitExport.h"
#include "Socket.h"

namespace sakit
{
	class ConnectorThread;

	class sakitExport TcpSocket : public Socket
	{
	public:
		TcpSocket(SocketDelegate* socketDelegate);
		~TcpSocket();

		bool isConnecting();
		bool isConnected();
		bool isDisconnecting();

		void update(float timeSinceLastFrame);

		// TODOsock - make it work with chstr port as well
		bool connect(Ip host, unsigned short port);
		bool disconnect();
		int send(hstream* stream, int count = INT_MAX);
		int send(chstr data);
		int receive(hstream* stream, int count);

		// TODOsock - make it work with chstr port as well
		bool connectAsync(Ip host, unsigned short port);
		bool disconnectAsync();
		bool sendAsync(hstream* stream, int count = INT_MAX);
		bool sendAsync(chstr data);
		bool receiveAsync(int count);

	protected:
		ConnectorThread* thread;

		void _activateConnection(Ip host, unsigned short port);

		bool _checkConnectStatus(State socketState);
		bool _checkDisconnectStatus(State socketState, State senderState, State receiverState);
		bool _checkSendStatus(State socketState, State senderState);
		bool _checkReceiveStatus(State socketState, State receiverState);

	};

}
#endif
