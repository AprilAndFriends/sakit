/// @file
/// @version 1.2
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
	Socket::Socket(SocketDelegate* socketDelegate, State idleState) :
		SocketBase()
	{
		this->socketDelegate = socketDelegate;
		this->idleState = idleState;
		this->sender = new SenderThread(this->socket, &this->timeout, &this->retryFrequency);
	}

	Socket::~Socket()
	{
		this->sender->join();
		delete this->sender;
		if (this->receiver != NULL)
		{
			this->receiver->join();
			delete this->receiver;
		}
	}

	bool Socket::isSending()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == State::Sending || this->state == State::SendingReceiving);
	}

	bool Socket::isReceiving()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == State::Receiving || this->state == State::SendingReceiving);
	}

	void Socket::update(float timeDelta)
	{
		this->_updateSending();
		this->_updateReceiving();
	}

	void Socket::_updateSending()
	{
		int sentCount = 0;
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThreadResult(&this->sender->resultMutex);
		hmutex::ScopeLock lockThreadSentCount(&this->sender->sentCountMutex);
		if (this->sender->sentCount > 0)
		{
			sentCount = this->sender->sentCount;
			this->sender->sentCount = 0;
		}
		lockThreadSentCount.release();
		State result = this->sender->result;
		if (result == State::Running || result == State::Idle)
		{
			lockThreadResult.release();
			lock.release();
			if (sentCount > 0)
			{
				this->socketDelegate->onSent(this, sentCount);
			}
			return;
		}
		this->sender->result = State::Idle;
		this->state = (this->state == State::SendingReceiving ? State::Receiving : this->idleState);
		lockThreadResult.release();
		lock.release();
		if (sentCount > 0)
		{
			this->socketDelegate->onSent(this, sentCount);
		}
		// delegate calls
		if (result == State::Finished)
		{
			this->socketDelegate->onSendFinished(this);
		}
		else if (result == State::Failed)
		{
			this->socketDelegate->onSendFailed(this);
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
		this->state = (this->state == State::Receiving ? State::SendingReceiving : State::Sending);
		lock.release();
		int result = this->_sendDirect(stream, count);
		lock.acquire(&this->mutexState);
		this->state = (this->state == State::SendingReceiving ? State::Receiving : this->idleState);
		return result;
	}

	bool Socket::sendAsync(hstream* stream, int count)
	{
		if (!this->_checkSendParameters(stream, count))
		{
			return false;
		}
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThreadResult(&this->sender->resultMutex);
		if (!this->_canSend(this->state))
		{
			return false;
		}
		this->state = (this->state == State::Receiving ? State::SendingReceiving : State::Sending);
		this->sender->result = State::Running;
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
				this->state = (this->state == State::Sending ? State::SendingReceiving : State::Receiving);
			}
		}
		return result;
	}

	int Socket::_finishReceive(int result)
	{
		hmutex::ScopeLock lock(&this->mutexState);
		this->state = (this->state == State::SendingReceiving ? State::Sending : this->idleState);
		return result;
	}

	bool Socket::_startReceiveAsync(int maxValue)
	{
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThreadResult(&this->receiver->resultMutex);
		if (!this->_canReceive(this->state))
		{
			return false;
		}
		this->state = (this->state == State::Sending ? State::SendingReceiving : State::Receiving);
		this->receiver->result = State::Running;
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
		this->receiver->executing = false;
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
		this->receiver->executing = false;
		return true;
	}

	
	bool Socket::_checkStartReceiveStatus(State receiverState)
	{
		if (receiverState == State::Running)
		{
			hlog::warn(logTag, "Cannot start receiving, already receiving!");
			return false;
		}
		return true;
	}

	bool Socket::_canSend(State state)
	{
		harray<State> allowed = State::allowedSendStatesBasic + this->idleState;
		return _checkState(state, allowed, "send");
	}

	bool Socket::_canReceive(State state)
	{
		harray<State> allowed = State::allowedReceiveStatesBasic + this->idleState;
		return _checkState(state, allowed, "receive");
	}

	bool Socket::_canStopReceive(State state)
	{
		return _checkState(state, State::allowedStopReceiveStates, "stop receiving");
	}

	bool Socket::_checkSendParameters(hstream* stream, int count)
	{
		if (stream == NULL)
		{
			hlog::warn(logTag, "Cannot send, stream is NULL!");
			return false;
		}
		if (stream->size() == 0)
		{
			hlog::warn(logTag, "Cannot send, no data to send!");
			return false;
		}
		if (count == 0)
		{
			hlog::warn(logTag, "Cannot send, count is 0!");
			return false;
		}
		return true;
	}

	bool Socket::_checkReceiveParameters(hstream* stream)
	{
		if (stream == NULL)
		{
			hlog::warn(logTag, "Cannot receive, stream is NULL!");
			return false;
		}
		return true;
	}

}
