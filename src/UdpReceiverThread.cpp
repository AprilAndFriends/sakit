/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SocketDelegate.h"
#include "UdpReceiverThread.h"

namespace sakit
{
	UdpReceiverThread::UdpReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : ReceiverThread(socket, timeout, retryFrequency)
	{
		this->name = "SAKit UDP receiver";
	}

	UdpReceiverThread::~UdpReceiverThread()
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

	void UdpReceiverThread::_updateProcess()
	{
		Host host;
		unsigned short port = 0;
		hstream* stream = new hstream();
		int count = this->maxValue;
		hmutex::ScopeLock lock;
		while (this->isRunning() && this->executing)
		{
			if (this->socket->receiveFrom(stream, host, port) && stream->size() > 0)
			{
				stream->rewind();
				lock.acquire(&this->mutex);
				this->remoteHosts += host;
				this->remotePorts += port;
				this->streams += stream;
				lock.release();
				host = Host();
				port = 0;
				stream = new hstream();
			}
			--count;
			if (this->maxValue > 0 && count == 0)
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		delete stream;
		lock.acquire(&this->mutex);
		this->result = FINISHED;
	}

}
