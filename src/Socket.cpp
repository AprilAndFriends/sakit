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
		this->sender = new SenderThread(this->socket);
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
		this->mutexState.lock();
		bool result = (this->state == SENDING || this->state == SENDING_RECEIVING);
		this->mutexState.unlock();
		return result;
	}

	bool Socket::isReceiving()
	{
		this->mutexState.lock();
		bool result = (this->state == RECEIVING || this->state == SENDING_RECEIVING);
		this->mutexState.unlock();
		return result;
	}

	void Socket::update(float timeSinceLastFrame)
	{
		this->_updateSending();
		this->_updateReceiving();
	}

	void Socket::_updateSending()
	{
		int sent = 0;
		this->mutexState.lock();
		this->sender->mutex.lock();
		if (this->sender->lastSent > 0)
		{
			sent = this->sender->lastSent;
			this->sender->lastSent = 0;
		}
		State state = this->state;
		State result = this->sender->result;
		if (result == RUNNING || result == IDLE)
		{
			this->sender->mutex.unlock();
			this->mutexState.unlock();
			if (sent > 0)
			{
				this->socketDelegate->onSent(this, sent);
			}
			return;
		}
		this->sender->result = IDLE;
		this->state = (this->state == SENDING_RECEIVING ? RECEIVING : this->idleState);
		this->sender->mutex.unlock();
		this->mutexState.unlock();
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
		return this->sendAsync(&stream, stream.size());
	}

	int Socket::_send(hstream* stream, int count)
	{
		if (!this->_checkSendParameters(stream, count))
		{
			return false;
		}
		this->mutexState.lock();
		State state = this->state;
		if (!this->_canSend(state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->state = (this->state == RECEIVING ? SENDING_RECEIVING : SENDING);
		this->mutexState.unlock();
		int result = this->_sendDirect(stream, count);
		this->mutexState.lock();
		this->state = (this->state == SENDING_RECEIVING ? RECEIVING : this->idleState);
		this->mutexState.unlock();
		return result;
	}

	bool Socket::sendAsync(hstream* stream, int count)
	{
		if (!this->_checkSendParameters(stream, count))
		{
			return false;
		}
		this->mutexState.lock();
		this->sender->mutex.lock();
		State state = this->state;
		if (!this->_canSend(state))
		{
			this->sender->mutex.unlock();
			this->mutexState.unlock();
			return false;
		}
		this->state = (this->state == RECEIVING ? SENDING_RECEIVING : SENDING);
		this->sender->result = RUNNING;
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)count, stream->size() - stream->position()));
		this->sender->stream->rewind();
		this->sender->mutex.unlock();
		this->mutexState.unlock();
		this->sender->start();
		return true;
	}

	bool Socket::_startReceiveAsync(int maxValue)
	{
		this->mutexState.lock();
		this->receiver->mutex.lock();
		State state = this->state;
		if (!this->_canReceive(state))
		{
			this->receiver->mutex.unlock();
			this->mutexState.unlock();
			return false;
		}
		this->state = (this->state == SENDING ? SENDING_RECEIVING : RECEIVING);
		this->receiver->result = RUNNING;
		this->receiver->maxValue = maxValue;
		this->receiver->mutex.unlock();
		this->mutexState.unlock();
		this->receiver->start();
		return true;
	}

	bool Socket::stopReceiveAsync()
	{
		this->mutexState.lock();
		State state = this->state;
		if (!this->_canStopReceive(state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->receiver->running = false;
		this->mutexState.unlock();
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
