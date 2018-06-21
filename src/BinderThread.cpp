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
#include "sakit.h"
#include "Socket.h"
#include "SocketDelegate.h"
#include "BinderThread.h"
#include "TcpSocket.h"

namespace sakit
{
	BinderThread::BinderThread(PlatformSocket* socket) :
		WorkerThread(socket)
	{
	}

	BinderThread::~BinderThread()
	{
	}

	void BinderThread::_updateBinding()
	{
		bool result = this->socket->bind(this->host, this->port);
		hmutex::ScopeLock lock(&this->resultMutex);
		this->result = (result ? State::Finished : State::Failed);
	}

	void BinderThread::_updateUnbinding()
	{
		bool result = this->socket->disconnect();
		hmutex::ScopeLock lock(&this->resultMutex);
		this->result = (result ? State::Finished : State::Failed);
	}

	void BinderThread::_updateProcess()
	{
		if (this->state == State::Binding)
		{
			this->_updateBinding();
		}
		else if (this->state == State::Unbinding)
		{
			this->_updateUnbinding();
		}
	}

}
