/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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
	ConnectorThread::ConnectorThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency), localPort(0)
	{
		this->name = "SAKit connector";
	}

	ConnectorThread::~ConnectorThread()
	{
	}

	void ConnectorThread::_updateConnecting()
	{
		Host localHost;
		unsigned short localPort = 0;
		bool result = this->socket->connect(this->host, this->port, localHost, localPort, *this->timeout, *this->retryFrequency);
		hmutex::ScopeLock lock(&this->mutex);
		if (result)
		{
			this->result = FINISHED;
			this->localHost = localHost;
			this->localPort = localPort;
		}
		else
		{
			this->result = FAILED;
		}
	}

	void ConnectorThread::_updateDisconnecting()
	{
		bool result = this->socket->disconnect();
		hmutex::ScopeLock lock(&this->mutex);
		this->result = (result ? FINISHED : FAILED);
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
		default:
			break;
		}
	}

}
