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
#include "SenderThread.h"
#include "Socket.h"
#include "SocketDelegate.h"

namespace sakit
{
	Socket::Socket(SocketDelegate* socketDelegate) : Base()
	{
		this->socketDelegate = socketDelegate;
		this->sender = new SenderThread(this->socket);
		this->receiver = new ReceiverThread(this->socket);
	}

	Socket::~Socket()
	{
		// TODOsock - check if this is all that is needed
		this->sender->running = false;
		this->sender->join();
		delete this->sender;
		this->receiver->running = false;
		this->receiver->join();
		delete this->receiver;
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

	void Socket::send(hsbase* stream, int maxBytes)
	{
		if (!this->isConnected())
		{
			hlog::error(sakit::logTag, "Not connected!");
			return;
		}
		this->sender->mutex.lock();
		if (this->sender->state != WorkerThread::IDLE)
		{
			hlog::warn(sakit::logTag, "Cannot send, already sending data!");
			this->sender->mutex.unlock();
			return;
		}
		this->sender->state = WorkerThread::RUNNING;
		long position = stream->position();
		stream->rewind();
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)maxBytes, stream->size()));
		this->sender->stream->rewind();
		this->sender->mutex.unlock();
		this->sender->start();
		stream->seek(position, hsbase::START);
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
		if (this->receiver->state != WorkerThread::IDLE)
		{
			hlog::warn(sakit::logTag, "Cannot receive, already receiving data!");
			this->receiver->mutex.unlock();
			return;
		}
		this->receiver->state = WorkerThread::RUNNING;
		this->receiver->mutex.unlock();
		this->receiver->maxBytes = maxBytes;
		this->receiver->start();
	}

	void Socket::update(float timeSinceLastFrame)
	{
		this->_updateSending();
		this->_updateReceiving();
	}

	void Socket::_updateSending()
	{
		this->sender->mutex.lock();
		WorkerThread::State state = this->sender->state;
		if (this->sender->lastSent > 0)
		{
			int sent = this->sender->lastSent;
			this->sender->lastSent = 0;
			this->sender->mutex.unlock();
			this->socketDelegate->onSent(this, sent);
		}
		else
		{
			this->sender->mutex.unlock();
		}
		if (state == WorkerThread::RUNNING || state == WorkerThread::IDLE)
		{
			return;
		}
		this->sender->mutex.lock();
		this->sender->state = WorkerThread::IDLE;
		this->sender->mutex.unlock();
		if (state == WorkerThread::FINISHED)
		{
			this->socketDelegate->onSendFinished(this);
		}
		else if (state == WorkerThread::FAILED)
		{
			this->socketDelegate->onSendFailed(this);
		}
	}
	
	void Socket::_updateReceiving()
	{
		this->receiver->mutex.lock();
		WorkerThread::State state = this->receiver->state;
		if (this->receiver->stream->size() > 0)
		{
			hstream* stream = this->receiver->stream;
			this->receiver->stream = new hstream();
			this->receiver->mutex.unlock();
			stream->rewind();
			this->socketDelegate->onReceived(this, stream);
			delete stream;
		}
		else
		{
			this->receiver->mutex.unlock();
		}
		if (state == WorkerThread::RUNNING || state == WorkerThread::IDLE)
		{
			return;
		}
		this->receiver->mutex.lock();
		this->receiver->state = WorkerThread::IDLE;
		this->receiver->mutex.unlock();
		if (state == WorkerThread::FINISHED)
		{
			this->socketDelegate->onReceiveFinished(this);
		}
		else if (state == WorkerThread::FAILED)
		{
			this->socketDelegate->onReceiveFailed(this);
		}
	}
	
}
