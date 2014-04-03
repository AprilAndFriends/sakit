/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdlib.h>

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SocketDelegate.h"
#include "BroadcasterThread.h"

namespace sakit
{
	BroadcasterThread::BroadcasterThread(PlatformSocket* socket) : WorkerThread(socket), remotePort(0)
	{
		this->stream = new hstream();
	}

	BroadcasterThread::~BroadcasterThread()
	{
		delete this->stream;
	}

	void BroadcasterThread::_updateProcess()
	{
		if (!this->socket->broadcast(this->adapters, this->remotePort, this->stream, this->stream->size()))
		{
			this->mutex.lock();
			this->result = FAILED;
			this->stream->clear();
			this->mutex.unlock();
			return;
		}
		this->mutex.lock();
		this->result = FINISHED;
		this->stream->clear();
		this->mutex.unlock();
	}

}
