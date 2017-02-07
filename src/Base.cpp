/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Base.h"
#include "PlatformSocket.h"
#include "sakit.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;
	extern hmutex updateMutex;

	void Base::__register()
	{
		hmutex::ScopeLock lock(&connectionsMutex);
		connections += this;
	}

	void Base::__unregister()
	{
		hmutex::ScopeLock lockUpdate(&updateMutex); // prevents deletion while update is still running
		hmutex::ScopeLock lock(&connectionsMutex);
		int index = connections.indexOf(this);
		if (index >= 0)
		{
			connections.removeAt(index);
		}
	}

	Base::Base() : state(State::Idle), localPort(0)
	{
		this->socket = new PlatformSocket();
		this->timeout = sakit::getGlobalTimeout();
		this->retryFrequency = sakit::getGlobalRetryFrequency();
	}

	Base::~Base()
	{
		delete this->socket;
	}

	void Base::setTimeout(float timeout, float retryFrequency)
	{
		this->timeout = timeout;
		this->retryFrequency = hmin(retryFrequency, timeout); // frequency can't be larger than the timeout itself
	}

	int Base::_sendDirect(hstream* stream, int count)
	{
		int sent = 0;
		int64_t position = stream->position();
		while (count > 0)
		{
			if (!this->socket->send(stream, count, sent))
			{
				break;
			}
			if (stream->eof())
			{
				break;
			}
			hthread::sleep(this->retryFrequency * 1000.0f);
		}
		stream->seek(position, hstream::START);
		return sent;
	}
	
	int Base::_receiveDirect(hstream* stream, int maxCount)
	{
		float time = 0.0f;
		int remainingCount = maxCount;
		int64_t position = stream->position();
		int64_t lastPosition = position;
		while (true)
		{
			if (!this->socket->receive(stream, remainingCount))
			{
				break;
			}
			if (maxCount > 0 && remainingCount == 0)
			{
				break;
			}
			if (lastPosition != stream->position())
			{
				lastPosition = stream->position();
				// retry attempts are reset after a successful read
				time = 0.0f;
				continue;
			}
			if (remainingCount != maxCount || lastPosition != position)
			{
				break;
			}
			time += this->retryFrequency;
			if (time >= this->timeout)
			{
				break;
			}
			hthread::sleep(this->retryFrequency * 1000.0f);
		}
		lastPosition = stream->position();
		if (time >= this->timeout)
		{
			hlog::warn(logTag, "Timed out while waiting for data.");
		}
		return (int)(lastPosition - position);
	}

	int Base::_receiveFromDirect(hstream* stream, Host& host, unsigned short& port)
	{
		float time = 0.0f;
		while (true)
		{
			if (this->socket->receiveFrom(stream, host, port))
			{
				break;
			}
			time += this->retryFrequency;
			if (time >= this->timeout)
			{
				break;
			}
			hthread::sleep(this->retryFrequency * 1000.0f);
		}
		return (int)stream->size();
	}

}
