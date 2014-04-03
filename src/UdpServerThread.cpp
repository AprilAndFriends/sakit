/// @file
/// @author  Boris Mikic
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
	}

	UdpServerThread::~UdpServerThread()
	{
		this->mutex.lock();
		foreach (hstream*, it, this->streams)
		{
			delete (*it);
		}
		this->remoteHosts.clear();
		this->remotePorts.clear();
		this->streams.clear();
		this->mutex.unlock();
	}

	void UdpServerThread::_updateProcess()
	{
		Host remoteHost;
		unsigned short remotePort = 0;
		hstream* stream = new hstream();
		while (this->running)
		{
			if (this->socket->receiveFrom(stream, remoteHost, remotePort))
			{
				if (stream->size() > 0)
				{
					stream->rewind();
					this->mutex.lock();
					this->remoteHosts += remoteHost;
					this->remotePorts += remotePort;
					this->streams += stream;
					this->mutex.unlock();
					remoteHost = Host();
					remotePort = 0;
					stream = new hstream();
				}
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		delete stream;
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

}
