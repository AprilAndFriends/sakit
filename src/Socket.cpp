/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "ReceiverThread.h"
#include "sakit.h"
#include "Socket.h"

namespace sakit
{
	Socket::Socket(ReceiverDelegate* receiverDelegate) : host("")
	{
		this->receiverDelegate = receiverDelegate;
		this->socket = new PlatformSocket();
		this->receiver = new ReceiverThread(this->socket, this->receiverDelegate);
	}

	Socket::~Socket()
	{
		// TODOsock - update this to work properly
		this->receiver->mutex.lock();
		this->receiver->stop();
		this->receiver->mutex.unlock();
		delete this->receiver;
		delete this->socket;
	}

	hstr Socket::getFullHost()
	{
		return (this->isConnected() ? hsprintf("%s:%d", this->host.getAddress().c_str(), this->port) : "");
	}

	bool Socket::isConnected()
	{
		return this->socket->isConnected();
	}

	bool Socket::connect(Ip host, unsigned short port)
	{
		if (this->disconnect()) // disconnect first
		{
			hlog::warn(sakit::logTag, "Connection already existed, it was closed.");
		}
		bool result = this->socket->connect(host, port);
		if (result)
		{
			this->host = host;
			this->port = port;
		}
		return result;
	}
	
	bool Socket::disconnect()
	{
		this->host = Ip("");
		this->port = 0;
		return this->socket->disconnect();
	}

	void Socket::receive(int maxBytes)
	{
		if (!this->isConnected())
		{
			hlog::error(sakit::logTag, "Not connected!");
			return;
		}
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot receive, maxBytes is 0!");
			return;
		}
		this->receiver->mutex.lock();
		if (this->receiver->running)
		{
			hlog::warn(sakit::logTag, "Cannot receive, already receiving data!");
			this->receiver->mutex.unlock();
			return;
		}
		this->receiver->mutex.unlock();
		this->receiver->maxBytes = maxBytes;
		this->receiver->execute();
		/*
		this->receiver = new hthread(&
		hstream data;
		this->socket->receive(data, maxBytes);
		int size = data.size();
		if (size > 0)
		{
			data.rewind();
			stream.write_raw(data);
			if (retainPosition)
			{
				stream.seek(-size);
			}
		}
		*/
		// TODOsock - call delegate here?
		//return size;
	}
	/*
	long Socket::receive(hsbase& stream, bool retainPosition)
	{
		return this->receive(stream, INT_MAX, retainPosition);
	}
	
	long Socket::receive(hsbase& stream, int maxBytes, bool retainPosition)
	{
		if (!this->isConnected())
		{
			hlog::error(sakit::logTag, "Not connected!");
			return 0;
		}
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot read: maxBytes is 0!");
			return 0;
		}
		hstream data;
		this->socket->receive(data, maxBytes);
		int size = data.size();
		if (size > 0)
		{
			data.rewind();
			stream.write_raw(data);
			if (retainPosition)
			{
				stream.seek(-size);
			}
		}
		// TODOsock - call delegate here?
		return size;
	}
	*/
	
}
