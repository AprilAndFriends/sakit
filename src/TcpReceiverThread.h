/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a thread for receiving data through TCP.

#ifndef SAKIT_TCP_RECEIVER_THREAD_H
#define SAKIT_TCP_RECEIVER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "ReceiverThread.h"

namespace sakit
{
	class PlatformSocket;
	class TcpSocket;

	class TcpReceiverThread : public ReceiverThread
	{
	public:
		friend class TcpSocket;

		TcpReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~TcpReceiverThread();

	protected:
		hstream* stream;
		hmutex streamMutex;

		void _updateProcess();

	};

}
#endif
