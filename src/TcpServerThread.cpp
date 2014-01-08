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
#include "TcpServerThread.h"
#include "TcpSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;

	TcpServerThread::TcpServerThread(PlatformSocket* socket, SocketDelegate* acceptedDelegate) :
		ServerThread(socket, acceptedDelegate)
	{
	}

	TcpServerThread::~TcpServerThread()
	{
	}

	void TcpServerThread::_updateRunning()
	{
		TcpSocket* socket = NULL;
		while (this->running)
		{
			if (!this->socket->listen())
			{
				this->mutex.lock();
				this->result = FAILED;
				this->mutex.unlock();
				return;
			}
			socket = new TcpSocket(this->acceptedDelegate);
			connectionsMutex.lock();
			connections -= socket;
			connectionsMutex.unlock();
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
		this->result = FINISHED;
		this->mutex.unlock();
	}

}
