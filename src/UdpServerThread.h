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
/// Defines a thread for handling a UDP server.

#ifndef SAKIT_UDP_SERVER_THREAD_H
#define SAKIT_UDP_SERVER_THREAD_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>

#include "Server.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class UdpServer;

	class UdpServerThread : public WorkerThread
	{
	public:
		friend class UdpServer;

		UdpServerThread(PlatformSocket* socket);
		~UdpServerThread();

	protected:
		harray<Host> remoteHosts;
		harray<unsigned short> remotePorts;
		harray<hstream*> streams;

		void _updateProcess();

	};

}
#endif
