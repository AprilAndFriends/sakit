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
#include "SocketDelegate.h"
#include "TcpReceiverThread.h"

namespace sakit
{
	TcpReceiverThread::TcpReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : ReceiverThread(socket, timeout, retryFrequency)
	{
		this->name = "SAKit TCP receiver";
		this->stream = new hstream();
	}

	TcpReceiverThread::~TcpReceiverThread()
	{
		delete this->stream;
	}

	void TcpReceiverThread::_updateProcess()
	{
		int remaining = this->maxValue;
		hmutex::ScopeLock lock;
		while (this->running)
		{
			if (!this->socket->receive(this->stream, this->mutex, remaining))
			{
				lock.acquire(&this->mutex);
				this->result = FAILED;
				return;
			}
			if (this->maxValue > 0 && remaining == 0)
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		lock.acquire(&this->mutex);
		this->result = FINISHED;
	}

}
