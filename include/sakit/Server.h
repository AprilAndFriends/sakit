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
/// Defines a generic server.

#ifndef SAKIT_SERVER_H
#define SAKIT_SERVER_H

#include <hltypes/hltypesUtil.h>

#include "Base.h"
#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class AccepterThread;
	class ServerDelegate;
	class Socket;
	class SocketDelegate;

	class sakitExport Server : public Base
	{
	public:
		Server(ServerDelegate* serverDelegate, SocketDelegate* socketDelegate);
		~Server();

		HL_DEFINE_GET(harray<Socket*>, sockets, Sockets);
		bool isBound();

		// TODOsock - allow chstr port as well?
		bool bind(Ip host, unsigned short port);
		bool unbind();
		void start();
		void stop();

		void update(float timeSinceLastFrame);

	protected:
		ServerDelegate* serverDelegate;
		SocketDelegate* socketDelegate;
		AccepterThread* accepter;
		harray<Socket*> sockets;

	};

}
#endif
