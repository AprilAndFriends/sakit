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

#include "sakitExport.h"
#include "IpSocket.h"

namespace sakit
{
	class ConnectorThread;

	class sakitExport TcpSocket : public IpSocket
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
