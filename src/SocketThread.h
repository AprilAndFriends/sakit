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
/// Defines a thread for handling socket-related functionality.

#ifndef SAKIT_SOCKET_THREAD_H
#define SAKIT_SOCKET_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Server.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;
	class TcpSocket;
	class UdpSocket;

	class SocketThread : public WorkerThread
	{
	public:
		friend class Socket;
		friend class TcpSocket;
		friend class UdpSocket;

		SocketThread(PlatformSocket* socket);
		~SocketThread();

	protected:
		SocketBase::State state;

	};

}
#endif
