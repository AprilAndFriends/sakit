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
#include "Socket.h"
#include "SocketDelegate.h"
#include "AccepterThread.h"

namespace sakit
{
	AccepterThread::AccepterThread(PlatformSocket* socket, SocketDelegate* socketDelegate) :
		WorkerThread(&process, socket), maxConnections(20)
	{
		this->socketDelegate = socketDelegate;
	}

	AccepterThread::~AccepterThread()
	{
	}

	void AccepterThread::_updateProcess()
	{
		Socket* socket = NULL;
		while (this->running)
		{
			if (!this->socket->listen(this->maxConnections))
			{
				this->mutex.lock();
				this->state = FAILED;
				this->mutex.unlock();
				break;
			}
			socket = new Socket(this->socketDelegate);
			if (!this->socket->accept(socket))
			{
				delete socket;
				hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
				continue;
			}
			this->mutex.lock();
			this->sockets += socket;
			this->mutex.unlock();
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		this->mutex.lock();
		this->state = FINISHED;
		this->mutex.unlock();
	}

	void AccepterThread::process(hthread* thread)
	{
		((AccepterThread*)thread)->_updateProcess();
	}

}
