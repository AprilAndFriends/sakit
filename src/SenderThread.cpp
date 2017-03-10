/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdlib.h>

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SocketDelegate.h"
#include "SenderThread.h"

namespace sakit
{
	SenderThread::SenderThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency), sentCount(0)
	{
		this->name = "SAKit sender";
		this->stream = new hstream();
	}

	SenderThread::~SenderThread()
	{
		delete this->stream;
	}

	void SenderThread::_updateProcess()
	{
		int count = (int)this->stream->size();
		int sent = 0;
		hmutex::ScopeLock lock;
		while (this->isRunning() && this->executing)
		{
			sent = 0;
			if (!this->socket->send(this->stream, count, sent))
			{
				lock.acquire(&this->resultMutex);
				this->result = State::Failed;
				lock.release();
				this->stream->clear();
				return;
			}
			lock.acquire(&this->sentCountMutex);
			this->sentCount += sent;
			lock.release();
			if (this->stream->eof())
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		lock.acquire(&this->resultMutex);
		this->result = State::Finished;
		lock.release();
		this->stream->clear();
	}

}
