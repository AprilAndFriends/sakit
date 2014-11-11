/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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
		bool result = this->socket->bind(this->host, this->port);
		hmutex::ScopeLock lock(&this->mutex);
		this->result = (result ? FINISHED : FAILED);
	}

	void BinderThread::_updateUnbinding()
	{
		bool result = this->socket->disconnect();
		hmutex::ScopeLock lock(&this->mutex);
		this->result = (result ? FINISHED : FAILED);
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
