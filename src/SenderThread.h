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
/// Defines a thread for sending data.

#ifndef SAKIT_SENDER_THREAD_H
#define SAKIT_SENDER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;
	class TcpSocket;
	class UdpSocket;

	class SenderThread : public WorkerThread
	{
	public:
		friend class Socket;
		friend class TcpSocket;
		friend class UdpSocket;

		SenderThread(PlatformSocket* socket);
		~SenderThread();

	protected:
		hstream* stream;
		int lastSent;

		void _updateProcess();

	};

}
#endif
