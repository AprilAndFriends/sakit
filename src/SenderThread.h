/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a thread for sending data.

#ifndef SAKIT_SENDER_THREAD_H
#define SAKIT_SENDER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "TimedThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;

	class SenderThread : public TimedThread
	{
	public:
		friend class Socket;

		SenderThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~SenderThread();

	protected:
		hstream* stream;
		int lastSent;

		void _updateProcess();

	};

}
#endif
