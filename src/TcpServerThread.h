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
/// Defines a thread for handling a TCP server.

#ifndef SAKIT_TCP_SERVER_THREAD_H
#define SAKIT_TCP_SERVER_THREAD_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>

#include "Server.h"
#include "TimedThread.h"

namespace sakit
{
	class PlatformSocket;
	class TcpServer;
	class TcpSocket;
	class TcpSocketDelegate;

	class TcpServerThread : public TimedThread
	{
	public:
		friend class TcpServer;

		TcpServerThread(PlatformSocket* socket, TcpSocketDelegate* acceptedDelegate, float* timeout, float* retryFrequency);
		~TcpServerThread();

	protected:
		TcpSocketDelegate* acceptedDelegate;
		harray<TcpSocket*> sockets;
		hmutex socketsMutex;

		void _updateProcess();

	};

}
#endif
