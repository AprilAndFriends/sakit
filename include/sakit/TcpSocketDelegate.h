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
/// Defines a TCP socket delegate.

#ifndef SAKIT_TCP_SOCKET_DELEGATE_H
#define SAKIT_TCP_SOCKET_DELEGATE_H

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"
#include "SocketDelegate.h"

namespace sakit
{
	class Socket;

	class sakitExport TcpSocketDelegate : public SocketDelegate
	{
	public:
		TcpSocketDelegate();
		virtual ~TcpSocketDelegate();

		virtual void onConnected(Socket* socket) = 0;
		virtual void onDisconnected(Socket* socket, Host host, unsigned short port) = 0;
		virtual void onConnectFailed(Socket* socket, Host host, unsigned short port) = 0;
		virtual void onDisconnectFailed(Socket* socket) = 0;

		virtual void onReceived(Socket* socket, hstream* stream) = 0;
		virtual void onReceiveFinished(Socket* socket) = 0;
		virtual void onReceiveFailed(Socket* socket) = 0;

	};

}
#endif
