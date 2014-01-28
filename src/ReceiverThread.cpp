/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "ReceiverThread.h"

namespace sakit
{
	ReceiverThread::ReceiverThread(PlatformSocket* socket) : WorkerThread(&process, socket),
		state(Socket::IDLE), maxBytes(0)
	{
		this->stream = new hstream();
	}

	ReceiverThread::~ReceiverThread()
	{
		delete this->stream;
	}

	void ReceiverThread::process(hthread* thread)
	{
		((ReceiverThread*)thread)->_updateProcess();
	}

}
