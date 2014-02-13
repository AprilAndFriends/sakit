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

#include "ConnectorThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"
#include "SocketDelegate.h"
#include "State.h"

namespace sakit
{
	ConnectorThread::ConnectorThread(PlatformSocket* socket) : WorkerThread(socket), localPort(0)
	{
	}

	ConnectorThread::~ConnectorThread()
	{
	}

	void ConnectorThread::_updateConnecting()
	{
		Host localHost;
		unsigned short localPort = 0;
		if (!this->socket->connect(this->host, this->port, localHost, localPort, sakit::getRetryTimeout(), sakit::getRetryAttempts()))
		{
			this->mutex.lock();
			this->result = FAILED;
			this->mutex.unlock();
			return;
		}
		this->mutex.lock();
		this->result = FINISHED;
		this->localHost = localHost;
		this->localPort = localPort;
		this->mutex.unlock();
	}

	void ConnectorThread::_updateDisconnecting()
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

	void ConnectorThread::_updateProcess()
	{
		switch (this->state)
		{
		case CONNECTING:
			this->_updateConnecting();
			break;
		case DISCONNECTING:
			this->_updateDisconnecting();
			break;
		}
	}

}
