/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SocketDelegate.h"
#include "ReceiverThread.h"

namespace sakit
{
	ReceiverThread::ReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency), maxValue(0)
	{
		this->name = "SAKit receiver";
	}

	ReceiverThread::~ReceiverThread()
	{
	}

}
