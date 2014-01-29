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
/// Defines a UDP socket delegate.

#ifndef SAKIT_UDP_SOCKET_DELEGATE_H
#define SAKIT_UDP_SOCKET_DELEGATE_H

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"
#include "SocketDelegate.h"

namespace sakit
{
	class Socket;

	class sakitExport UdpSocketDelegate : public SocketDelegate
	{
	public:
		UdpSocketDelegate();
		virtual ~UdpSocketDelegate();

		virtual void onReceived(Socket* socket, Host host, unsigned short port, hstream* stream);

	};

}
#endif
