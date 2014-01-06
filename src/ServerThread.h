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
/// Defines a thread for binding a server.

#ifndef SAKIT_SERVER_THREAD_H
#define SAKIT_SERVER_THREAD_H

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

	class sakitExport ServerThread : public WorkerThread
	{
	public:
		friend class Server;

		ServerThread(PlatformSocket* socket, ServerDelegate* serverDelegate, SocketDelegate* acceptedDelegate);
		~ServerThread();

	protected:
		Server::State state;
		ServerDelegate* serverDelegate;
		SocketDelegate* acceptedDelegate;
		harray<Socket*> sockets;
		Ip host;
		unsigned short port;

		void _updateBinding();
		void _updateRunning();
		void _updateUnbinding();
		void _updateProcess();

		static void process(hthread*);

	};

}
#endif
