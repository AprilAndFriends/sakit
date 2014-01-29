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

#include "Server.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class TcpServer;
	class TcpSocket;
	class TcpSocketDelegate;

	class TcpServerThread : public WorkerThread
	{
	public:
		friend class TcpServer;

		TcpServerThread(PlatformSocket* socket, TcpSocketDelegate* acceptedDelegate);
		~TcpServerThread();

	protected:
		TcpSocketDelegate* acceptedDelegate;
		harray<TcpSocket*> sockets;

		void _updateProcess();

		void _updateBinding();
		void _updateUnbinding();
		void _updateRunning();

	};

}
#endif
