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
#include "SocketDelegate.h"
#include "ReceiverThread.h"

namespace sakit
{
	ReceiverThread::ReceiverThread(PlatformSocket* socket) : WorkerThread(&process, socket),
		state(Socket::IDLE), maxBytes(INT_MAX)
	{
		this->stream = new hstream();
	}

	ReceiverThread::~ReceiverThread()
	{
		delete this->stream;
	}

	void ReceiverThread::_updateProcess()
	{
		while (this->running)
		{
			if (!this->socket->receive(this->stream, this->mutex, this->maxBytes))
			{
				this->mutex.lock();
				this->result = FAILED;
				this->mutex.unlock();
				return;
			}
			if (this->maxBytes == 0)
			{
				break;
			}
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

	void ReceiverThread::process(hthread* thread)
	{
		((ReceiverThread*)thread)->_updateProcess();
	}

}
