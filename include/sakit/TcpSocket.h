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

#include "Connector.h"
#include "Host.h"
#include "sakitExport.h"
#include "Socket.h"
#include "State.h"

namespace sakit
{
	class ConnectorThread;
	class TcpReceiverThread;
	class TcpSocketDelegate;

	class sakitExport TcpSocket : public Socket, public Connector
	{
	public:
		TcpSocket(TcpSocketDelegate* socketDelegate);
		~TcpSocket();

		bool setNagleAlgorithmActive(bool value);

		void update(float timeSinceLastFrame = 0.0f);

		/// @note Keep in mind that only all queued stream data is received at once.
		int receive(hstream* stream, int maxBytes = 0);
		bool startReceiveAsync(int maxBytes = 0);

	protected:
		TcpSocketDelegate* tcpSocketDelegate;
		TcpReceiverThread* tcpReceiver;

		void _updateReceiving();

		void _activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort);

	};

}
#endif
