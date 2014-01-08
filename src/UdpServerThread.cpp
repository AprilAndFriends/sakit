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
#include "UdpServerThread.h"
#include "UdpSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;

	UdpServerThread::UdpServerThread(PlatformSocket* socket) : ServerThread(socket)
	{
	}

	UdpServerThread::~UdpServerThread()
	{
	}

	void UdpServerThread::_updateRunning()
	{
		hstream* stream = new hstream();
		Ip host("");
		unsigned short port = 0;
		while (this->running)
		{
			if (!this->socket->receiveFrom(stream, &host, &port))
			{
				delete stream;
				this->mutex.lock();
				this->result = FAILED;
				this->mutex.unlock();
				return;
			}
			if (stream->size() > 0)
			{
				this->mutex.lock();
				this->streams += stream;
				this->hosts += host;
				this->ports += port;
				this->mutex.unlock();
				stream = new hstream();
			}
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		delete stream;
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

}
