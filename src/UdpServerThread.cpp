/// @file
/// @version 1.0
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
#include "UdpServerThread.h"
#include "UdpSocket.h"

namespace sakit
{
	UdpServerThread::UdpServerThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency)
	{
		this->name = "SAKit UDP server";
	}

	UdpServerThread::~UdpServerThread()
	{
		hmutex::ScopeLock lock(&this->mutex);
		foreach (hstream*, it, this->streams)
		{
			delete (*it);
		}
		this->remoteHosts.clear();
		this->remotePorts.clear();
		this->streams.clear();
	}

	void UdpServerThread::_updateProcess()
	{
		Host remoteHost;
		unsigned short remotePort = 0;
		hstream* stream = new hstream();
		hmutex::ScopeLock lock;
		while (this->executing)
		{
			if (this->socket->receiveFrom(stream, remoteHost, remotePort))
			{
				if (stream->size() > 0)
				{
					stream->rewind();
					lock.acquire(&this->mutex);
					this->remoteHosts += remoteHost;
					this->remotePorts += remotePort;
					this->streams += stream;
					lock.release();
					remoteHost = Host();
					remotePort = 0;
					stream = new hstream();
				}
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		delete stream;
		lock.acquire(&this->mutex);
		this->result = FINISHED;
	}

}
