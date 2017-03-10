/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a UDP socket delegate.

#ifndef SAKIT_UDP_SOCKET_DELEGATE_H
#define SAKIT_UDP_SOCKET_DELEGATE_H

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "BinderDelegate.h"
#include "Host.h"
#include "sakitExport.h"
#include "SocketDelegate.h"

namespace sakit
{
	class UdpSocket;

	class sakitExport UdpSocketDelegate : public SocketDelegate, public BinderDelegate
	{
	public:
		UdpSocketDelegate();
		virtual ~UdpSocketDelegate();

		virtual void onReceived(UdpSocket* socket, Host remoteHost, unsigned short remotePort, hstream* stream);

		virtual void onBroadcastFinished(UdpSocket* socket);
		virtual void onBroadcastFailed(UdpSocket* socket);

	};

}
#endif
