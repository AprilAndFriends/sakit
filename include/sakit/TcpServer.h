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
/// Defines a TCP server.

#ifndef SAKIT_TCP_SERVER_H
#define SAKIT_TCP_SERVER_H

#include "sakitExport.h"
#include "Server.h"

namespace sakit
{
	class PlatformSocket;
	class SocketDelegate;

	class sakitExport TcpServer : public Server
	{
	public:
		TcpServer(ServerDelegate* serverDelegate, SocketDelegate* socketDelegate, int maxConnections = 20);
		~TcpServer();

	};

}
#endif
