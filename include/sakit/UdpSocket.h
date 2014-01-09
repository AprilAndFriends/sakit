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
/// Defines a socket working over UDP.

#ifndef SAKIT_UDP_SOCKET_H
#define SAKIT_UDP_SOCKET_H

#include <hltypes/hstream.h>

#include "Ip.h"
#include "sakitExport.h"
#include "Socket.h"

namespace sakit
{
	class ConnectorThread;

	class sakitExport UdpSocket : public Socket
	{
	public:
		UdpSocket(SocketDelegate* socketDelegate);
		~UdpSocket();

		bool hasDestination();

		bool setDestination(Ip host, unsigned short port);
		bool clearDestination();
		int send(hstream* stream, int count = INT_MAX);
		int send(chstr data);
		int receive(hstream* stream, int count);

		bool sendAsync(hstream* stream, int count = INT_MAX);
		bool sendAsync(chstr data);
		bool receiveAsync(int count);

	protected:
		void _activateConnection(Ip host, unsigned short port);

	};

}
#endif
