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
#include "Ip.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"
#include "SocketDelegate.h"

namespace sakit
{
	ConnectorThread::ConnectorThread(PlatformSocket* socket) : WorkerThread(&process, socket), state(Socket::IDLE)
	{
	}

	ConnectorThread::~ConnectorThread()
	{
	}

	void ConnectorThread::_updateConnecting()
	{
		if (!this->socket->connect(this->host, this->port))
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
		case Socket::CONNECTING:
			this->_updateConnecting();
			break;
		case Socket::DISCONNECTING:
			this->_updateDisconnecting();
			break;
		}
	}

	void ConnectorThread::process(hthread* thread)
	{
		((ConnectorThread*)thread)->_updateProcess();
	}

}
