/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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

	void Base::__register()
	{
		connectionsMutex.lock();
		connections += this;
		connectionsMutex.unlock();
	}

	void Base::__unregister()
	{
		connectionsMutex.lock();
		int index = connections.index_of(this);
		if (index >= 0)
		{
			connections.remove_at(index);
		}
		connectionsMutex.unlock();
	}

	Base::Base() : localPort(0), state(IDLE)
	{
		this->socket = new PlatformSocket();
	}

	Base::~Base()
	{
		delete this->socket;
	}

	int Base::_sendDirect(hstream* stream, int count)
	{
		int sent = 0;
		long position = stream->position();
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
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
			hthread::sleep(retryTimeout);
		}
		stream->seek(position, hstream::START);
		return sent;
	}
	
	int Base::_receiveDirect(hstream* stream, int maxBytes)
	{
		hmutex mutex;
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
		int retryAttempts = sakit::getRetryAttempts();
		int remaining = maxBytes;
		int position = stream->position();
		int lastPosition = position;
		while (retryAttempts > 0)
		{
			if (!this->socket->receive(stream, mutex, remaining))
			{
				break;
			}
			if (maxBytes > 0 && remaining == 0)
			{
				break;
			}
			if (lastPosition != stream->position())
			{
				lastPosition = stream->position();
				// retry attempts are reset after a successful read
				retryAttempts = sakit::getRetryAttempts();
				continue;
			}
			if (remaining != maxBytes || lastPosition != position)
			{
				break;
			}
			retryAttempts--;
			hthread::sleep(retryTimeout);
		}
		lastPosition = stream->position();
		if (retryAttempts == 0)
		{
			hlog::warn(logTag, "Timed out while waiting for data.");
		}
		return (lastPosition - position);
	}

	int Base::_receiveFromDirect(hstream* stream, Host& host, unsigned short& port)
	{
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
		int retryAttempts = sakit::getRetryAttempts();
		while (retryAttempts > 0)
		{
			if (this->socket->receiveFrom(stream, host, port))
			{
				break;
			}
			retryAttempts--;
			hthread::sleep(retryTimeout);
		}
		return stream->size();
	}

}
