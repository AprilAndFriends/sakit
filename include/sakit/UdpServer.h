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
	class UdpServerDelegate;
	class UdpServerThread;
	class UdpSocket;

	class sakitExport UdpServer : public Server
	{
	public:
		UdpServer(UdpServerDelegate* serverDelegate);
		~UdpServer();

		void update(float timeSinceLastFrame = 0.0f);

		bool receive(hstream* stream, Host& remoteHost, unsigned short& remotePort, float timeout = 0.0f);

	protected:
		UdpServerThread* udpServerThread;
		UdpServerDelegate* udpServerDelegate;

	};

}
#endif
