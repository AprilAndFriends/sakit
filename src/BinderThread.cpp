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
#include "BinderThread.h"
#include "TcpSocket.h"

namespace sakit
{
	BinderThread::BinderThread(PlatformSocket* socket) : WorkerThread(socket)
	{
	}

	BinderThread::~BinderThread()
	{
	}

	void BinderThread::_updateBinding()
	{
		if (!this->socket->bind(this->host, this->port))
		{
			this->mutex.lock();
			this->result = FAILED;
			this->mutex.unlock();
			return;
		}
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

	void BinderThread::_updateUnbinding()
	{
		if (!this->socket->disconnect())
		{
			this->mutex.lock();
			this->result = FAILED;
			this->mutex.unlock();
			return;
		}
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

	void BinderThread::_updateProcess()
	{
		switch (this->state)
		{
		case BINDING:
			this->_updateBinding();
			break;
		case UNBINDING:
			this->_updateUnbinding();
			break;
		}
	}

}
