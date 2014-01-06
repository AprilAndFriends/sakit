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

#include "Ip.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"
#include "SocketDelegate.h"
#include "SocketThread.h"

namespace sakit
{
	SocketThread::SocketThread(PlatformSocket* socket) : WorkerThread(&process, socket), state(Socket::IDLE)
	{
	}

	SocketThread::~SocketThread()
	{
	}

	void SocketThread::_updateConnecting()
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

	void SocketThread::_updateDisconnecting()
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

	void SocketThread::_updateProcess()
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

	void SocketThread::process(hthread* thread)
	{
		((SocketThread*)thread)->_updateProcess();
	}

}
