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
/// Defines a thread for handling a TCP server.

#ifndef SAKIT_TCP_SERVER_THREAD_H
#define SAKIT_TCP_SERVER_THREAD_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>

#include "sakitExport.h"
#include "Server.h"
#include "ServerThread.h"

namespace sakit
{
	class PlatformSocket;
	class SocketDelegate;
	class TcpServer;
	class TcpSocket;

	class sakitExport TcpServerThread : public ServerThread
	{
	public:
		friend class Server;
		friend class TcpServer;

		TcpServerThread(PlatformSocket* socket, SocketDelegate* acceptedDelegate);
		~TcpServerThread();

	protected:
		TcpServerThread* thread;
		harray<TcpSocket*> sockets;

		void _updateRunning();

	};

}
#endif
