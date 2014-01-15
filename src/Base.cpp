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

	Base::Base() : port(0)
	{
		connectionsMutex.lock();
		connections += this;
		connectionsMutex.unlock();
		this->socket = new PlatformSocket();
	}

	Base::~Base()
	{
		delete this->socket;
		connectionsMutex.lock();
		int index = connections.index_of(this);
		if (index >= 0)
		{
			connections.remove_at(index);
		}
		connectionsMutex.unlock();
	}

	hstr Base::getFullHost()
	{
		return (this->socket->isConnected() ? hsprintf("%s:%d", this->host.toString().c_str(), this->port) : "");
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
		while (remaining == maxBytes && retryAttempts > 0)
		{
			if (!this->socket->receive(stream, mutex, remaining))
			{
				break;
			}
			if (maxBytes == 0)
			{
				break;
			}
			retryAttempts--;
			hthread::sleep(retryTimeout);
		}
		return (maxBytes - remaining);
	}

	bool Base::_canSend(hstream* stream, int count)
	{
		if (stream == NULL)
		{
			hlog::warn(sakit::logTag, "Cannot send, stream is NULL!");
			return false;
		}
		if (stream->size() == 0)
		{
			hlog::warn(sakit::logTag, "Cannot send, no data to send!");
			return false;
		}
		if (count == 0)
		{
			hlog::warn(sakit::logTag, "Cannot send, count is 0!");
			return false;
		}
		return true;
	}

	bool Base::_canReceive(hstream* stream)
	{
		if (stream == NULL)
		{
			hlog::warn(sakit::logTag, "Cannot receive, stream is NULL!");
			return false;
		}
		return true;
	}

	void Base::_activateConnection(Host host, unsigned short port)
	{
		this->host = host;
		this->port = port;
	}

}
