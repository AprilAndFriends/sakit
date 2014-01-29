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

#include "ConnectorDelegate.h"
#include "Host.h"
#include "sakitExport.h"
#include "SocketDelegate.h"

namespace sakit
{
	class Socket;

	class sakitExport TcpSocketDelegate : public SocketDelegate, public ConnectorDelegate
	{
	public:
		TcpSocketDelegate();
		~TcpSocketDelegate();

		virtual void onReceived(Socket* socket, hstream* stream);
		virtual void onReceiveFailed(Socket* socket);

	};

}
#endif
