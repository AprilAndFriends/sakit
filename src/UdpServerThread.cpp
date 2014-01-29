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
	UdpServerThread::UdpServerThread(PlatformSocket* socket) : WorkerThread(socket)
	{
	}

	UdpServerThread::~UdpServerThread()
	{
		this->mutex.lock();
		foreach (hstream*, it, this->streams)
		{
			delete (*it);
		}
		this->hosts.clear();
		this->ports.clear();
		this->streams.clear();
		this->mutex.lock();
	}

	void UdpServerThread::_updateProcess()
	{
		Host host;
		unsigned short port = 0;
		hstream* stream = new hstream();
		while (this->running)
		{
			if (this->socket->receiveFrom(stream, host, port))
			{
				if (stream->size() > 0)
				{
					stream->rewind();
					this->mutex.lock();
					this->hosts += host;
					this->ports += port;
					this->streams += stream;
					this->mutex.unlock();
					host = Host();
					port = 0;
					stream = new hstream();
				}
			}
			hthread::sleep(sakit::getRetryTimeout() * 1000.0f);
		}
		delete stream;
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

}
