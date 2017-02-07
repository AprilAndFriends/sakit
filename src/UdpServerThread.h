/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a thread for handling a UDP server.

#ifndef SAKIT_UDP_SERVER_THREAD_H
#define SAKIT_UDP_SERVER_THREAD_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>

#include "Server.h"
#include "TimedThread.h"

namespace sakit
{
	class PlatformSocket;
	class UdpServer;

	class UdpServerThread : public TimedThread
	{
	public:
		friend class UdpServer;

		UdpServerThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~UdpServerThread();

	protected:
		harray<Host> remoteHosts;
		harray<unsigned short> remotePorts;
		harray<hstream*> streams;

		void _updateProcess();

	};

}
#endif
