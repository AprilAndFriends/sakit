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

#include "Host.h"
#include "NetworkAdapter.h"
#include "sakitExport.h"
#include "Socket.h"

namespace sakit
{
	class ConnectorThread;
	class UdpReceiverThread;
	class UdpSocketDelegate;

	class sakitExport UdpSocket : public Socket
	{
	public:
		UdpSocket(UdpSocketDelegate* socketDelegate);
		~UdpSocket();

		bool hasMulticastGroup() { return this->multicastGroup; }
		bool hasDestination();

		bool setMulticastInterface(Host address);
		bool setMulticastTtl(int value);
		bool setMulticastLoopback(bool value);

		bool setDestination(Host host, unsigned short port);
		bool clearDestination();

		/// @note Keep in mind that only one datagram is received at the time.
		int receive(hstream* stream, Host& host, unsigned short& port);
		bool startReceiveAsync(int maxPackages = 0);

		bool joinMulticastGroup(Host address, unsigned short port, Host groupAddress);

		static bool broadcast(unsigned short port, hstream* stream, int count = INT_MAX);
		static bool broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count = INT_MAX);
		static bool broadcast(unsigned short port, chstr data);
		static bool broadcast(harray<NetworkAdapter> adapters, unsigned short port, chstr data);

	protected:
		UdpSocketDelegate* udpSocketDelegate;
		UdpReceiverThread* udpReceiver;
		bool multicastGroup;

		void _updateReceiving();

		void _activateConnection(Host host, unsigned short port);

	};

}
#endif
