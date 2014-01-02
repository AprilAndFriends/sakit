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
/// Defines a thread for accepting incoming connections.

#ifndef SAKIT_ACCEPTER_THREAD_H
#define SAKIT_ACCEPTER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "sakitExport.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class Server;
	class Socket;
	class SocketDelegate;

	class sakitExport AccepterThread : public WorkerThread
	{
	public:
		friend class Server;

		AccepterThread(PlatformSocket* socket, SocketDelegate* socketDelegate);
		~AccepterThread();

	protected:
		SocketDelegate* socketDelegate;
		harray<Socket*> sockets;
		int maxConnections;

		void _updateProcess();

		static void process(hthread*);

	};

}
#endif
