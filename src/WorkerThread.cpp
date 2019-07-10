/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "WorkerThread.h"

namespace sakit
{
	WorkerThread::WorkerThread(PlatformSocket* socket) :
		hthread(&process, "SAKit worker"),
		result(State::Idle),
		port(0)
	{
		this->socket = socket;
	}

	void WorkerThread::process(hthread* thread)
	{
		((WorkerThread*)thread)->_updateProcess();
	}

}
