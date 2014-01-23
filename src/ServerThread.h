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
/// Defines a thread for handling a server.

#ifndef SAKIT_SERVER_THREAD_H
#define SAKIT_SERVER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Server.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;

	class ServerThread : public WorkerThread
	{
	public:
		friend class Server;

		ServerThread(PlatformSocket* socket, SocketDelegate* acceptedDelegate);
		~ServerThread();

	protected:
		Server::State state;
		SocketDelegate* acceptedDelegate;

		void _updateBinding();
		void _updateUnbinding();
		void _updateProcess();

		virtual void _updateRunning() = 0;

		static void process(hthread*);

	};

}
#endif
