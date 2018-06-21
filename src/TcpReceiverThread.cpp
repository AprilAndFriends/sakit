/// @file
/// @version 1.2
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
	TcpReceiverThread::TcpReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency) :
		ReceiverThread(socket, timeout, retryFrequency)
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
		while (this->isRunning() && this->executing)
		{
			if (!this->socket->receive(this->stream, remaining, &this->streamMutex))
			{
				lock.acquire(&this->resultMutex);
				this->result = State::Failed;
				return;
			}
			if (this->maxValue > 0 && remaining == 0)
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		lock.acquire(&this->resultMutex);
		this->result = State::Finished;
	}

}
