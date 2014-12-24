/// @file
/// @version 1.0
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
	SenderThread::SenderThread(PlatformSocket* socket, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency), lastSent(0)
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
		while (this->running)
		{
			sent = 0;
			if (!this->socket->send(this->stream, count, sent))
			{
				lock.acquire(&this->mutex);
				this->result = FAILED;
				this->stream->clear();
				return;
			}
			lock.acquire(&this->mutex);
			this->lastSent += sent;
			lock.release();
			if (this->stream->eof())
			{
				break;
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		lock.acquire(&this->mutex);
		this->result = FINISHED;
		this->stream->clear();
	}

}
