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
/// Defines a thread for receiving data.

#ifndef SAKIT_RECEIVER_THREAD_H
#define SAKIT_RECEIVER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;
	class SocketBase;
	class TcpSocket;
	class UdpSocket;

	class ReceiverThread : public WorkerThread
	{
	public:
		friend class Socket;
		friend class SocketBase;
		friend class TcpSocket;
		friend class UdpSocket;

		ReceiverThread(PlatformSocket* socket);
		~ReceiverThread();

	protected:
		SocketBase::State state;
		hstream* stream;
		int maxBytes;

		void _updateProcess();

		static void process(hthread*);

	};

}
#endif
