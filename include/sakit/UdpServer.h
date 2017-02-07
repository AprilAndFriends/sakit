/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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

		void update(float timeDelta = 0.0f);

		bool receive(hstream* stream, Host& remoteHost, unsigned short& remotePort);

	protected:
		UdpServerThread* udpServerThread;
		UdpServerDelegate* udpServerDelegate;

	private:
		UdpServer(const UdpServer& other); // prevents copying

	};

}
#endif
