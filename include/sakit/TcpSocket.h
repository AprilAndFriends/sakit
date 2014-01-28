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

#include "Host.h"
#include "sakitExport.h"
#include "Socket.h"

namespace sakit
{
	class ConnectorThread;
	class TcpReceiverThread;
	class TcpSocketDelegate;

	class sakitExport TcpSocket : public Socket
	{
	public:
		TcpSocket(TcpSocketDelegate* socketDelegate);
		~TcpSocket();

		bool isConnecting();
		bool isConnected();
		bool isDisconnecting();

		bool setNagleAlgorithmActive(bool value);

		void update(float timeSinceLastFrame);

		// TODOsock - make it work with chstr port as well
		bool connect(Host host, unsigned short port);
		bool disconnect();
		/// @note Keep in mind that only all queued stream data is received at once.
		int receive(hstream* stream, int maxBytes = 0);

		// TODOsock - make it work with chstr port as well
		bool connectAsync(Host host, unsigned short port);
		bool disconnectAsync();
		bool startReceiveAsync(int maxBytes = 0);

	protected:
		TcpSocketDelegate* tcpSocketDelegate;
		TcpReceiverThread* tcpReceiver;
		ConnectorThread* thread;

		void _updateReceiving();

		int _send(hstream* stream, int count);
		bool _sendAsync(hstream* stream, int count);

		void _activateConnection(Host host, unsigned short port);

		bool _checkConnectStatus(State socketState);
		bool _checkConnectedStatus(State socketState, chstr action);
		bool _checkDisconnectStatus(State socketState, State senderState, State receiverState);
		bool _checkSendStatus(State socketState, State senderState);
		bool _checkStartReceiveStatus(State socketState, State receiverState);

	};

}
#endif
