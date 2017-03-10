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
/// Defines a UDP server delegate.

#ifndef SAKIT_UDP_SERVER_DELEGATE_H
#define SAKIT_UDP_SERVER_DELEGATE_H

#include <hltypes/hstream.h>

#include "sakitExport.h"
#include "ServerDelegate.h"

namespace sakit
{
	class UdpServer;
	class UdpSocket;

	class sakitExport UdpServerDelegate : public ServerDelegate
	{
	public:
		UdpServerDelegate();
		~UdpServerDelegate();

		virtual void onReceived(UdpServer* server, Host remoteHost, unsigned short remotePort, hstream* stream);

	};

}
#endif
