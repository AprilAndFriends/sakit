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

#include "Binder.h"
#include "Host.h"
#include "NetworkAdapter.h"
#include "sakitExport.h"
#include "Socket.h"

namespace sakit
{
	class BroadcasterThread;
	class UdpReceiverThread;
	class UdpSocketDelegate;

	class sakitExport UdpSocket : public Socket, public Binder
	{
	public:
		UdpSocket(UdpSocketDelegate* socketDelegate);
		~UdpSocket();

		HL_DEFINE_GET2(harray<std::pair, Host, Host>, multicastHosts, MulticastHosts);
		bool hasDestination();

		bool setMulticastInterface(Host interfaceHost);
		bool setMulticastTtl(int value);
		bool setMulticastLoopback(bool value);

		void update(float timeSinceLastFrame = 0.0f);

		bool setDestination(Host remoteHost, unsigned short remotePort);

		/// @note Keep in mind that only one datagram is received at the time.
		int receive(hstream* stream, Host& remoteHost, unsigned short& remotePort);
		bool startReceiveAsync(int maxPackages = 0);

		bool broadcast(harray<NetworkAdapter> adapters, unsigned short remotePort, hstream* stream, int count = INT_MAX);
		bool broadcast(unsigned short remotePort, hstream* stream, int count = INT_MAX);
		bool broadcast(harray<NetworkAdapter> adapters, unsigned short remotePort, chstr data);
		bool broadcast(unsigned short remotePort, chstr data);
		bool broadcastAsync(harray<NetworkAdapter> adapters, unsigned short remotePort, hstream* stream, int count = INT_MAX);
		bool broadcastAsync(unsigned short remotePort, hstream* stream, int count = INT_MAX);
		bool broadcastAsync(harray<NetworkAdapter> adapters, unsigned short remotePort, chstr data);
		bool broadcastAsync(unsigned short remotePort, chstr data);
		
		bool joinMulticastGroup(Host interfaceHost, Host groupAddress);
		bool leaveMulticastGroup(Host interfaceHost, Host groupAddress);

	protected:
		UdpSocketDelegate* udpSocketDelegate;
		UdpReceiverThread* udpReceiver;
		BroadcasterThread* broadcaster;
		harray<std::pair<Host, Host> > multicastHosts;

		void _updateReceiving();
		void _clear();
		void _activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort);

		bool _canSetDestination(State state);
		bool _canJoinMulticastGroup(State state);
		bool _canLeaveMulticastGroup(State state);

	};

}
#endif
