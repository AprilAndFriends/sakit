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
		this->sender->running = false;
		this->sender->join();
		delete this->sender;
		this->receiver->running = false;
		this->receiver->join();
		delete this->receiver;
	}

	bool Socket::isSending()
	{
		this->sender->mutex.lock();
		bool result = (this->sender->state == RUNNING);
		this->sender->mutex.unlock();
		return result;
	}

	bool Socket::isReceiving()
	{
		this->receiver->mutex.lock();
		bool result = (this->receiver->state == RUNNING);
		this->receiver->mutex.unlock();
		return result;
	}

	void Socket::update(float timeSinceLastFrame)
	{
		this->_updateSending();
		this->_updateReceiving();
	}

	void Socket::_updateSending()
	{
		this->sender->mutex.lock();
		WorkerThread::Result result = this->sender->result;
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
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			return;
		}
		this->sender->mutex.lock();
		this->sender->result = WorkerThread::IDLE;
		this->sender->state = IDLE;
		this->sender->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			this->socketDelegate->onSendFinished(this);
		}
		else if (result == WorkerThread::FAILED)
		{
			this->socketDelegate->onSendFailed(this);
		}
	}

	void Socket::_updateReceiving()
	{
		this->receiver->mutex.lock();
		WorkerThread::Result result = this->receiver->result;
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
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			return;
		}
		this->receiver->mutex.lock();
		this->receiver->result = WorkerThread::IDLE;
		this->receiver->state = IDLE;
		this->receiver->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			this->socketDelegate->onReceiveFinished(this);
		}
		else if (result == WorkerThread::FAILED)
		{
			this->socketDelegate->onReceiveFailed(this);
		}
	}

	int Socket::send(chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->send(&stream);
	}

	bool Socket::sendAsync(chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->sendAsync(&stream);
	}

	bool Socket::stopReceiveAsync()
	{
		this->receiver->mutex.lock();
		State receiverState = this->receiver->state;
		if (!this->_checkStopReceiveStatus(receiverState))
		{
			this->receiver->mutex.unlock();
			return false;
		}
		this->receiver->running = false;
		this->receiver->mutex.unlock();
		return true;
	}

	bool Socket::_checkSendStatus(State senderState)
	{
		if (senderState == RUNNING)
		{
			hlog::warn(sakit::logTag, "Cannot send, already sending!");
			return false;
		}
		return true;
	}

	bool Socket::_checkStartReceiveStatus(State receiverState)
	{
		if (receiverState == RUNNING)
		{
			hlog::warn(sakit::logTag, "Cannot start receiving, already receiving!");
			return false;
		}
		return true;
	}

	bool Socket::_checkStopReceiveStatus(State receiverState)
	{
		if (receiverState == IDLE)
		{
			hlog::warn(sakit::logTag, "Cannot stop receiving, not receiving!");
			return false;
		}
		return true;
	}

}
