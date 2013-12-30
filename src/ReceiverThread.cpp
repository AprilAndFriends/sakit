/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdlib.h>

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "ReceiverDelegate.h"
#include "ReceiverThread.h"

namespace sakit
{
	ReceiverThread::ReceiverThread(PlatformSocket* socket) : hthread(&process), maxBytes(INT_MAX), hasError(false)
	{
		this->socket = socket;
		this->stream = new hstream();
	}

	ReceiverThread::~ReceiverThread()
	{
		this->mutex.lock();
		delete this->stream;
		this->mutex.unlock();
	}

	void ReceiverThread::process(hthread* thread)
	{
		ReceiverThread* receiverThread = (ReceiverThread*)thread;
		while (receiverThread->running && receiverThread->maxBytes > 0)
		{
			if (!receiverThread->socket->receive(receiverThread->stream, receiverThread->mutex, receiverThread->maxBytes))
			{
				receiverThread->running = false;
				receiverThread->hasError = true;
				break;
			}
			hthread::sleep(100.0f); // TODOsock - make this configurable?
		}
	}

}
