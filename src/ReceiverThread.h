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
/// Defines a generic thread for receiving data.

#ifndef SAKIT_RECEIVER_THREAD_H
#define SAKIT_RECEIVER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "TimedThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;

	class ReceiverThread : public TimedThread
	{
	public:
		friend class Socket;

		ReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~ReceiverThread();

	protected:
		int maxValue;

	};

}
#endif
