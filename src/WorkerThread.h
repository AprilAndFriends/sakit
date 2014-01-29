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
/// Defines a thread for executing async socket work.

#ifndef SAKIT_WORKER_THREAD_H
#define SAKIT_WORKER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "Host.h"
#include "SocketBase.h"
#include "State.h"

namespace sakit
{
	class PlatformSocket;
	class Server;
	class Socket;
	class TcpSocket;
	class UdpSocket;
	
	class WorkerThread : public hthread
	{
	public:
		friend class Server;
		friend class Socket;
		friend class TcpSocket;
		friend class UdpSocket;

		WorkerThread(PlatformSocket* socket);
		~WorkerThread();

	protected:
		State _state; // TODOsock - remove

		State result;
		PlatformSocket* socket;
		Host host;
		unsigned short port;
		hmutex mutex;

		virtual void _updateProcess() = 0;

		static void process(hthread* thread);

	};

}
#endif
