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
/// Defines a thread for connecting/disconnecting a socket.

#ifndef SAKIT_SOCKET_THREAD_H
#define SAKIT_SOCKET_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Ip.h"
#include "sakitExport.h"
#include "Server.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class ServerDelegate;
	class SocketDelegate;

	class sakitExport SocketThread : public WorkerThread
	{
	public:
		friend class Socket;

		SocketThread(PlatformSocket* socket);
		~SocketThread();

	protected:
		Socket::State state;

		void _updateConnecting();
		void _updateDisconnecting();
		void _updateProcess();

		static void process(hthread*);

	};

}
#endif
