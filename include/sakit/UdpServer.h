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
/// Defines a UDP server.

#ifndef SAKIT_UDP_SERVER_H
#define SAKIT_UDP_SERVER_H

#include "sakitExport.h"
#include "Server.h"
#include "Socket.h"

namespace sakit
{
	class SenderThread;
	class SocketDelegate;
	class UdpServerDelegate;
	class UdpServerThread;
	class UdpSocket;

	class sakitExport UdpServer : public Server
	{
	public:
		UdpServer(UdpServerDelegate* serverDelegate, SocketDelegate* socketDelegate);
		~UdpServer();

		void update(float timeSinceLastFrame);

		int send(Ip host, unsigned short port, hstream* stream, int count = INT_MAX);
		int send(Ip host, unsigned short port, chstr data);
		bool receive(hstream* stream, Ip* host, unsigned short* port);

		bool sendAsync(Ip host, unsigned short port, hstream* stream, int count = INT_MAX);
		bool sendAsync(Ip host, unsigned short port, chstr data);

	protected:
		UdpServerThread* thread;
		SenderThread* sender;
		UdpServerDelegate* serverDelegate;

		bool _checkSendStatus(Socket::State senderState);

	};

}
#endif
