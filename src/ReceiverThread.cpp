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
	ReceiverThread::ReceiverThread(PlatformSocket* socket, ReceiverDelegate* receiverDelegate) :
		hthread(&process), maxBytes(INT_MAX)
	{
		this->socket = socket;
		this->receiverDelegate = receiverDelegate;
	}

	ReceiverThread::~ReceiverThread()
	{
	}

	void ReceiverThread::process(hthread* thread)
	{
		ReceiverThread* receiverThread = (ReceiverThread*)thread;
		while (receiverThread->running && receiverThread->maxBytes > 0)
		{
			hstream stream;
			if (receiverThread->socket->receive(stream, receiverThread->maxBytes) > 0)
			{
				stream.rewind();
				receiverThread->mutex.lock();
				receiverThread->receiverDelegate->onReceived(stream);
				receiverThread->mutex.unlock();
			}
		}

		//receiverThread->receiverDelegate;
	}

}
