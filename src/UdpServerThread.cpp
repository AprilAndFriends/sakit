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
#include "UdpServerThread.h"
#include "UdpSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;

	UdpServerThread::UdpServerThread(PlatformSocket* socket, SocketDelegate* acceptedDelegate) : ServerThread(socket, acceptedDelegate)
	{
	}

	UdpServerThread::~UdpServerThread()
	{
	}

	void UdpServerThread::_updateRunning()
	{
		UdpSocket* socket = NULL;
		hstream* stream = new hstream();
		while (this->running)
		{
			socket = new UdpSocket(this->acceptedDelegate);
			connectionsMutex.lock();
			connections -= socket;
			connectionsMutex.unlock();
			if (!this->socket->receiveFrom(stream, socket))
			{
				delete socket;
				hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
				continue;
			}
			if (stream->size() > 0)
			{
				this->mutex.lock();
				this->sockets += socket;
				this->streams += stream;
				this->mutex.unlock();
				stream = new hstream();
			}
			else
			{
				delete socket;
			}
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		delete stream;
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

}
