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
/// Defines a thread for broadcasting data.

#ifndef SAKIT_BROADCASTER_THREAD_H
#define SAKIT_BROADCASTER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>

#include "NetworkAdapter.h"
#include "Socket.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;

	class BroadcasterThread : public WorkerThread
	{
	public:
		friend class UdpSocket;

		BroadcasterThread(PlatformSocket* socket);
		~BroadcasterThread();

	protected:
		hstream* stream;
		harray<NetworkAdapter> adapters;
		unsigned short remotePort;

		void _updateProcess();

	};

}
#endif