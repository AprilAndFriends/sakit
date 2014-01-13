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
#include "NetworkAdapter.h"
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
		bool hasMulticastGroup() { return this->multicastGroup; }

		bool setDestination(Ip host, unsigned short port);
		bool clearDestination();

		int send(hstream* stream, int count = INT_MAX);
		int send(chstr data);
		int receive(hstream* stream, int count = 0);

		bool sendAsync(hstream* stream, int count = INT_MAX);
		bool sendAsync(chstr data);
		bool startReceiveAsync(int count = 0);
		
		bool joinMulticastGroup(Ip address, unsigned short port, Ip groupAddress);
		bool setMulticastInterface(Ip address);
		bool setMulticastTtl(int value);
		bool setMulticastLoopback(bool value);

		static bool broadcast(unsigned short port, hstream* stream, int count = INT_MAX);
		static bool broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count = INT_MAX);
		static bool broadcast(unsigned short port, chstr data);
		static bool broadcast(harray<NetworkAdapter> adapters, unsigned short port, chstr data);

	protected:
		bool multicastGroup;

		void _activateConnection(Ip host, unsigned short port);

	};

}
#endif
