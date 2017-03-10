/// @file
/// @version 1.2
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
		hmutex::ScopeLock lock(&this->resultMutex);
		if (result)
		{
			this->result = State::Finished;
			this->localHost = localHost;
			this->localPort = localPort;
		}
		else
		{
			this->result = State::Failed;
		}
	}

	void ConnectorThread::_updateDisconnecting()
	{
		bool result = this->socket->disconnect();
		hmutex::ScopeLock lock(&this->resultMutex);
		this->result = (result ? State::Finished : State::Failed);
	}

	void ConnectorThread::_updateProcess()
	{
		if (this->state == State::Connecting)
		{
			this->_updateConnecting();
		}
		else if (this->state == State::Disconnecting)
		{
			this->_updateDisconnecting();
		}
	}

}
