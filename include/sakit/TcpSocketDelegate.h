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
	class TcpSocket;

	class sakitExport TcpSocketDelegate : public SocketDelegate, public ConnectorDelegate
	{
	public:
		TcpSocketDelegate();
		~TcpSocketDelegate();

		virtual void onReceived(TcpSocket* socket, hstream* stream);
		virtual void onReceiveFailed(TcpSocket* socket);

	};

}
#endif
