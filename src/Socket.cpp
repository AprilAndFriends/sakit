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
#include "SocketThread.h"

namespace sakit
{
	Socket::Socket(SocketDelegate* socketDelegate) : Base()
	{
		this->socketDelegate = socketDelegate;
		this->thread = new SocketThread(this->socket);
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

	bool Socket::isConnecting()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == CONNECTING);
		this->thread->mutex.unlock();
		return result;
	}

	bool Socket::isConnected()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == CONNECTED || this->thread->state == DISCONNECTING);
		this->thread->mutex.unlock();
		return result;
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

	bool Socket::isDisconnecting()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == DISCONNECTING);
		this->thread->mutex.unlock();
		return result;
	}

	bool Socket::connect(Ip host, unsigned short port)
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		bool result = (this->_checkConnectStatus(state) && this->socket->connect(host, port));
		if (result)
		{
			this->thread->mutex.lock();
			this->thread->state = CONNECTED;
			this->thread->mutex.unlock();
			this->host = host;
			this->port = port;
		}
		return result;
	}
	
	bool Socket::disconnect()
	{
		this->thread->mutex.lock();
		this->sender->mutex.lock();
		this->receiver->mutex.lock();
		State socketState = this->thread->state;
		State senderState = this->sender->state;
		State receiverState = this->receiver->state;
		this->receiver->mutex.unlock();
		this->sender->mutex.unlock();
		this->thread->mutex.unlock();
		bool result = (this->_checkDisconnectStatus(socketState, senderState, receiverState) && this->socket->disconnect());
		if (result)
		{
			this->thread->mutex.lock();
			this->thread->state = IDLE;
			this->thread->mutex.unlock();
			this->host = Ip("");
			this->port = 0;
		}
		return result;
	}

	int Socket::send(hsbase* stream, int maxBytes)
	{
		if (stream->size() == 0)
		{
			hlog::warn(sakit::logTag, "Cannot send, no data to send!");
			return false;
		}
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot send, maxBytes is 0!");
			return false;
		}
		this->thread->mutex.lock();
		this->sender->mutex.lock();
		State socketState = this->thread->state;
		State senderState = this->sender->state;
		this->sender->mutex.unlock();
		this->thread->mutex.unlock();
		if (!this->_checkSendStatus(socketState, senderState))
		{
			return false;
		}
		int sent = 0;
		long position = stream->position();
		stream->rewind();
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
		while (maxBytes > 0)
		{
			if (!this->socket->send(stream, sent, maxBytes))
			{
				break;
			}
			if (stream->eof())
			{
				break;
			}
			hthread::sleep(retryTimeout);
		}
		stream->seek(position, hsbase::START);
		return sent;
	}

	hsbase* Socket::receive(int maxBytes)
	{
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot receive, maxBytes is 0!");
			return NULL;
		}
		this->thread->mutex.lock();
		this->receiver->mutex.lock();
		State socketState = this->thread->state;
		State receiverState = this->receiver->state;
		this->receiver->mutex.unlock();
		this->thread->mutex.unlock();
		if (!this->_checkReceiveStatus(socketState, receiverState))
		{
			return NULL;
		}
		hstream* stream = new hstream();
		hmutex mutex;
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
		int remaining = maxBytes;
		while (remaining > 0)
		{
			if (!this->socket->receive(stream, mutex, remaining))
			{
				break;
			}
			hthread::sleep(retryTimeout);
		}
		return stream;
	}

	bool Socket::connectAsync(Ip host, unsigned short port)
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		if (!this->_checkConnectStatus(state))
		{
			this->thread->mutex.unlock();
			return false;
		}
		this->thread->host = host;
		this->thread->port = port;
		this->thread->state = CONNECTING;
		this->thread->mutex.unlock();
		this->thread->start();
		return true;
	}
	
	bool Socket::disconnectAsync()
	{
		this->thread->mutex.lock();
		this->sender->mutex.lock();
		this->receiver->mutex.lock();
		State socketState = this->thread->state;
		State senderState = this->sender->state;
		State receiverState = this->receiver->state;
		if (!this->_checkDisconnectStatus(socketState, senderState, receiverState))
		{
			this->receiver->mutex.unlock();
			this->sender->mutex.unlock();
			this->thread->mutex.unlock();
			return false;
		}
		this->thread->state = DISCONNECTING;
		this->receiver->mutex.unlock();
		this->sender->mutex.unlock();
		this->thread->mutex.unlock();
		this->thread->start();
		return true;
	}

	bool Socket::sendAsync(hsbase* stream, int maxBytes)
	{
		if (stream->size() == 0)
		{
			hlog::warn(sakit::logTag, "Cannot send, no data to send!");
			return false;
		}
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot send, maxBytes is 0!");
			return false;
		}
		this->thread->mutex.lock();
		this->sender->mutex.lock();
		State socketState = this->thread->state;
		State senderState = this->sender->state;
		if (!this->_checkSendStatus(socketState, senderState))
		{
			this->sender->mutex.unlock();
			this->thread->mutex.unlock();
			return false;
		}
		long position = stream->position();
		stream->rewind();
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)maxBytes, stream->size()));
		this->sender->stream->rewind();
		this->sender->state = RUNNING;
		this->sender->mutex.unlock();
		this->thread->mutex.unlock();
		this->sender->start();
		stream->seek(position, hsbase::START);
		return true;
	}

	bool Socket::receiveAsync(int maxBytes)
	{
		if (maxBytes == 0)
		{
			hlog::warn(sakit::logTag, "Cannot receive, maxBytes is 0!");
			return false;
		}
		this->thread->mutex.lock();
		this->receiver->mutex.lock();
		State socketState = this->thread->state;
		State receiverState = this->receiver->state;
		if (!this->_checkReceiveStatus(socketState, receiverState))
		{
			this->receiver->mutex.unlock();
			this->thread->mutex.unlock();
			return false;
		}
		this->receiver->maxBytes = maxBytes;
		this->receiver->result = WorkerThread::RUNNING;
		this->receiver->mutex.unlock();
		this->thread->mutex.unlock();
		this->receiver->start();
		return true;
	}

	void Socket::update(float timeSinceLastFrame)
	{
		this->_updateSending();
		this->_updateReceiving();
		this->thread->mutex.lock();
		State state = this->thread->state;
		WorkerThread::Result result = this->thread->result;
		Ip host = this->thread->host;
		unsigned short port = this->thread->port;
		this->thread->mutex.unlock();
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			return;
		}
		this->thread->mutex.lock();
		this->thread->result = WorkerThread::IDLE;
		this->thread->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			switch (state)
			{
			case CONNECTING:
				this->thread->mutex.lock();
				this->thread->state = CONNECTED;
				this->thread->mutex.unlock();
				this->host = host;
				this->port = port;
				this->socketDelegate->onConnected(this);
				break;
			case DISCONNECTING:
				this->thread->mutex.lock();
				this->thread->state = IDLE;
				this->thread->mutex.unlock();
				host = this->host;
				port = this->port;
				this->host = Ip("");
				this->port = 0;
				this->socketDelegate->onDisconnected(this, host, port);
				break;
			}
		}
		else if (state == WorkerThread::FAILED)
		{
			switch (state)
			{
			case CONNECTING:
				this->thread->mutex.lock();
				this->thread->state = IDLE;
				this->thread->mutex.unlock();
				this->socketDelegate->onConnectFailed(this, host, port);
				break;
			case DISCONNECTING:
				this->thread->mutex.lock();
				this->thread->state = CONNECTED;
				this->thread->mutex.unlock();
				this->socketDelegate->onDisconnectFailed(this);
				break;
			}
		}
	}

	void Socket::_activateConnection(Ip host, unsigned short port)
	{
		Base::_activateConnection(host, port);
		this->thread->state = CONNECTED;
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
	
	bool Socket::_checkConnectStatus(State socketState)
	{
		switch (socketState)
		{
		case CONNECTING:	hlog::warn(sakit::logTag, "Cannot connect, already connecting!");		return false;
		case CONNECTED:		hlog::warn(sakit::logTag, "Cannot connect, already connected!");		return false;
		case DISCONNECTING:	hlog::warn(sakit::logTag, "Cannot connect, already disconnecting!");	return false;
		}
		return true;
	}

	bool Socket::_checkSendStatus(State socketState, State senderState)
	{
		switch (socketState)
		{
		case IDLE:			hlog::warn(sakit::logTag, "Cannot send, not connected!");			return false;
		case CONNECTING:	hlog::warn(sakit::logTag, "Cannot send, still connecting!");		return false;
		case DISCONNECTING:	hlog::warn(sakit::logTag, "Cannot send, already disconnecting!");	return false;
		}
		switch (senderState)
		{
		case RUNNING:	hlog::warn(sakit::logTag, "Cannot send, already sending!");	return false;
		}
		return true;
	}

	bool Socket::_checkReceiveStatus(State socketState, State receiverState)
	{
		switch (socketState)
		{
		case IDLE:			hlog::warn(sakit::logTag, "Cannot receive, not connected!");			return false;
		case CONNECTING:	hlog::warn(sakit::logTag, "Cannot receive, still connecting!");			return false;
		case DISCONNECTING:	hlog::warn(sakit::logTag, "Cannot receive, already disconnecting!");	return false;
		}
		switch (receiverState)
		{
		case RUNNING:	hlog::warn(sakit::logTag, "Cannot receive, already receiving!");	return false;
		}
		return true;
	}

	bool Socket::_checkDisconnectStatus(State socketState, State senderState, State receiverState)
	{
		switch (socketState)
		{
		case IDLE:			hlog::warn(sakit::logTag, "Cannot disconnect, not connected!");			return false;
		case CONNECTING:	hlog::warn(sakit::logTag, "Cannot disconnect, still connecting!");		return false;
		case DISCONNECTING:	hlog::warn(sakit::logTag, "Cannot disconnect, already disconnecting!");	return false;
		}
		bool result = true;
		if (senderState == RUNNING)
		{
			hlog::warn(sakit::logTag, "Cannot disconnect, still sending!");
			result = false;
		}
		if (receiverState == RUNNING)
		{
			hlog::warn(sakit::logTag, "Cannot disconnect, still receiving!");
			result = false;
		}
		return result;
	}

}
