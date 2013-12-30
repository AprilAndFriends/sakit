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
#include "ReceiverDelegate.h"
#include "ReceiverThread.h"
#include "sakit.h"
#include "Socket.h"

namespace sakit
{
	extern harray<Socket*> sockets;

	Socket::Socket(ReceiverDelegate* receiverDelegate) : host("")
	{
		sockets += this;
		this->receiverDelegate = receiverDelegate;
		this->socket = new PlatformSocket();
		this->receiver = new ReceiverThread(this->socket);
	}

	Socket::~Socket()
	{
		// TODOsock - update this to work properly
		this->receiver->mutex.lock();
		this->receiver->stop();
		this->receiver->mutex.unlock();
		delete this->receiver;
		delete this->socket;
		sockets -= this;
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
		if (this->receiver->state != ReceiverThread::IDLE)
		{
			hlog::warn(sakit::logTag, "Cannot receive, already receiving data!");
			this->receiver->mutex.unlock();
			return;
		}
		this->receiver->state = ReceiverThread::RUNNING;
		this->receiver->mutex.unlock();
		this->receiver->maxBytes = maxBytes;
		this->receiver->start();
	}

	void Socket::send(hsbase* stream)
	{
		if (!this->isConnected())
		{
			hlog::error(sakit::logTag, "Not connected!");
			return;
		}
		this->socket->send(stream);
	}

	void Socket::update(float timeSinceLastFrame)
	{
		this->receiver->mutex.lock();
		ReceiverThread::State state = this->receiver->state;
		this->receiver->mutex.unlock();
		if (state != ReceiverThread::RUNNING)
		{
			if (state == ReceiverThread::FINISHED)
			{
				this->receiver->mutex.lock();
				this->receiver->state = ReceiverThread::IDLE;
				this->receiver->mutex.unlock();
				this->receiverDelegate->onFinished(this);
			}
			else if (state == ReceiverThread::FAILED)
			{
				this->receiver->mutex.lock();
				this->receiver->state = ReceiverThread::IDLE;
				this->receiver->mutex.unlock();
				this->receiverDelegate->onFailed(this);
			}
			return;
		}
		this->receiver->mutex.lock();
		if (this->receiver->stream->size() == 0)
		{
			this->receiver->mutex.unlock();
			return;
		}
		hstream* stream = this->receiver->stream;
		this->receiver->stream = new hstream();
		this->receiver->mutex.unlock();
		stream->rewind();
		this->receiverDelegate->onReceived(this, stream);
		delete stream;
	}
	
}
