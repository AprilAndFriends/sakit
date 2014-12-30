/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "ReceiverThread.h"
#include "sakit.h"
#include "sakitUtil.h"
#include "SenderThread.h"
#include "Socket.h"
#include "SocketDelegate.h"
#include "State.h"

namespace sakit
{
	Socket::Socket(SocketDelegate* socketDelegate, State idleState) : SocketBase()
	{
		this->socketDelegate = socketDelegate;
		this->idleState = idleState;
		this->sender = new SenderThread(this->socket, &this->timeout, &this->retryFrequency);
	}

	Socket::~Socket()
	{
		this->sender->join();
		delete this->sender;
		this->receiver->join();
		delete this->receiver;
	}

	bool Socket::isSending()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == SENDING || this->state == SENDING_RECEIVING);
	}

	bool Socket::isReceiving()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == RECEIVING || this->state == SENDING_RECEIVING);
	}

	void Socket::update(float timeDelta)
	{
		this->_updateSending();
		this->_updateReceiving();
	}

	void Socket::_updateSending()
	{
		int sent = 0;
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->sender->mutex);
		if (this->sender->lastSent > 0)
		{
			sent = this->sender->lastSent;
			this->sender->lastSent = 0;
		}
		State result = this->sender->result;
		if (result == RUNNING || result == IDLE)
		{
			if (sent > 0)
			{
				lockThread.release();
				lock.release();
				this->socketDelegate->onSent(this, sent);
			}
			return;
		}
		this->sender->result = IDLE;
		this->state = (this->state == SENDING_RECEIVING ? RECEIVING : this->idleState);
		lockThread.release();
		lock.release();
		if (sent > 0)
		{
			this->socketDelegate->onSent(this, sent);
		}
		// delegate calls
		switch (result)
		{
		case FINISHED:	this->socketDelegate->onSendFinished(this);	break;
		case FAILED:	this->socketDelegate->onSendFailed(this);	break;
		}
	}

	int Socket::send(hstream* stream, int count)
	{
		return this->_send(stream, count);
	}

	int Socket::send(chstr data)
	{
		return SocketBase::_send(data);
	}
	
	bool Socket::sendAsync(chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->sendAsync(&stream, (int)stream.size());
	}

	int Socket::_send(hstream* stream, int count)
	{
		if (!this->_checkSendParameters(stream, count))
		{
			return false;
		}
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canSend(this->state))
		{
			return false;
		}
		this->state = (this->state == RECEIVING ? SENDING_RECEIVING : SENDING);
		lock.release();
		int result = this->_sendDirect(stream, count);
		lock.acquire(&this->mutexState);
		this->state = (this->state == SENDING_RECEIVING ? RECEIVING : this->idleState);
		return result;
	}

	bool Socket::sendAsync(hstream* stream, int count)
	{
		if (!this->_checkSendParameters(stream, count))
		{
			return false;
		}
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->sender->mutex);
		if (!this->_canSend(this->state))
		{
			return false;
		}
		this->state = (this->state == RECEIVING ? SENDING_RECEIVING : SENDING);
		this->sender->result = RUNNING;
		this->sender->stream->clear();
		this->sender->stream->writeRaw(*stream, (int)hmin((int64_t)count, stream->size() - stream->position()));
		this->sender->stream->rewind();
		this->sender->start();
		return true;
	}

	bool Socket::_prepareReceive(hstream* stream)
	{
		bool result = false;
		if (this->_checkReceiveParameters(stream))
		{
			hmutex::ScopeLock lock(&this->mutexState);
			result = this->_canReceive(this->state);
			if (result)
			{
				this->state = (this->state == SENDING ? SENDING_RECEIVING : RECEIVING);
			}
		}
		return result;
	}

	int Socket::_finishReceive(int result)
	{
		hmutex::ScopeLock lock(&this->mutexState);
		this->state = (this->state == SENDING_RECEIVING ? SENDING : this->idleState);
		return result;
	}

	bool Socket::_startReceiveAsync(int maxValue)
	{
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->receiver->mutex);
		if (!this->_canReceive(this->state))
		{
			return false;
		}
		this->state = (this->state == SENDING ? SENDING_RECEIVING : RECEIVING);
		this->receiver->result = RUNNING;
		this->receiver->maxValue = maxValue;
		this->receiver->start();
		return true;
	}

	bool Socket::stopReceive()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canStopReceive(this->state))
		{
			return false;
		}
		this->receiver->running = false;
		lock.release();
		this->receiver->join();
		this->_updateReceiving();
		return true;
	}
	
	bool Socket::stopReceiveAsync()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canStopReceive(this->state))
		{
			return false;
		}
		this->receiver->running = false;
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

	bool Socket::_canSend(State state)
	{
		harray<State> allowed;
		allowed += this->idleState;
		allowed += RECEIVING;
		return _checkState(state, allowed, "send");
	}

	bool Socket::_canReceive(State state)
	{
		harray<State> allowed;
		allowed += this->idleState;
		allowed += SENDING;
		return _checkState(state, allowed, "receive");
	}

	bool Socket::_canStopReceive(State state)
	{
		harray<State> allowed;
		allowed += RECEIVING;
		allowed += SENDING_RECEIVING;
		return _checkState(state, allowed, "stop receiving");
	}

	bool Socket::_checkSendParameters(hstream* stream, int count)
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

	bool Socket::_checkReceiveParameters(hstream* stream)
	{
		if (stream == NULL)
		{
			hlog::warn(sakit::logTag, "Cannot receive, stream is NULL!");
			return false;
		}
		return true;
	}

}
