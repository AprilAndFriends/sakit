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
#include "sakit.h"
#include "SocketDelegate.h"
#include "SenderThread.h"

namespace sakit
{
	SenderThread::SenderThread(PlatformSocket* socket) : WorkerThread(&process, socket), lastSent(0)
	{
	}

	SenderThread::~SenderThread()
	{
	}

	void SenderThread::_updateProcess()
	{
		while (this->running)
		{
			if (!this->socket->send(this->stream, this->lastSent))
			{
				this->mutex.lock();
				this->result = FAILED;
				this->stream->clear();
				this->mutex.unlock();
				return;
			}
			if (this->stream->position() >= this->stream->size())
			{
				break;
			}
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		this->mutex.lock();
		this->result = FINISHED;
		this->stream->clear();
		this->mutex.unlock();
	}

	void SenderThread::process(hthread* thread)
	{
		((SenderThread*)thread)->_updateProcess();
	}

}
