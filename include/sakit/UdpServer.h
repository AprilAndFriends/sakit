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
	class SocketDelegate;
	class UdpServerDelegate;
	class UdpServerThread;
	class UdpSocket;

	class sakitExport UdpServer : public Server
	{
	public:
		UdpServer(UdpServerDelegate* serverDelegate, SocketDelegate* receivedDelegate);
		~UdpServer();

		harray<UdpSocket*> getSockets();

		void update(float timeSinceLastFrame);

		UdpSocket* receive(hstream* stream, float timeout = 0.0f);

	protected:
		harray<UdpSocket*> sockets;
		UdpServerThread* thread;
		UdpServerDelegate* serverDelegate;
		SocketDelegate* receivedDelegate;

		void _updateSockets();

	};

}
#endif
