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
/// Defines an implementation for the delegate that is required for a TCP socket.

#ifndef SAKIT_HTTP_TCP_SOCKET_DELEGATE_H
#define SAKIT_HTTP_TCP_SOCKET_DELEGATE_H

#include <hltypes/hstream.h>

#include "Host.h"
#include "sakitExport.h"
#include "SocketDelegate.h"

namespace sakit
{
	class Socket;

	class HttpTcpSocketDelegate : public SocketDelegate
	{
	public:
		HttpTcpSocketDelegate();
		~HttpTcpSocketDelegate();

		void onConnected(sakit::Socket* socket);
		void onDisconnected(sakit::Socket* socket, sakit::Host host, unsigned short port);
		void onConnectFailed(sakit::Socket* socket, sakit::Host host, unsigned short port);
		void onDisconnectFailed(sakit::Socket* socket);

		void onSent(sakit::Socket* socket, int bytes);
		void onSendFinished(sakit::Socket* socket);
		void onSendFailed(sakit::Socket* socket);

		void onReceived(sakit::Socket* socket, hstream* stream);
		void onReceiveFinished(sakit::Socket* socket);
		void onReceiveFailed(sakit::Socket* socket);

	};

}
#endif
