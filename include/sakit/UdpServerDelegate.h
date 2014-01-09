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
/// Defines a UDP server delegate.

#ifndef SAKIT_UDP_SERVER_DELEGATE_H
#define SAKIT_UDP_SERVER_DELEGATE_H

#include <hltypes/hstream.h>

#include "Ip.h"
#include "sakitExport.h"
#include "ServerDelegate.h"

namespace sakit
{
	class UdpServer;

	class sakitExport UdpServerDelegate : public ServerDelegate
	{
	public:
		UdpServerDelegate();
		~UdpServerDelegate();

		virtual void onReceived(UdpServer* server, hstream* stream, Ip host, unsigned short port) = 0;

	};

}
#endif
