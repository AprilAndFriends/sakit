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
/// Defines a thread that has to know how long it may it can wait until a new processing attempt.

#ifndef SAKIT_TIMED_THREAD_H
#define SAKIT_TIMED_THREAD_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>

#include "Server.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;

	class TimedThread : public WorkerThread
	{
	public:
		TimedThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~TimedThread();

	protected:
		float* timeout;
		float* retryFrequency;

	};

}
#endif
