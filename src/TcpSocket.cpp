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
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "ConnectorThread.h"
#include "PlatformSocket.h"
#include "ReceiverThread.h"
#include "sakit.h"
#include "SenderThread.h"
#include "SocketDelegate.h"
#include "TcpSocket.h"

namespace sakit
{
	TcpSocket::TcpSocket(SocketDelegate* socketDelegate) : Socket(socketDelegate)
	{
		this->socket->setConnectionLess(false);
		this->thread = new ConnectorThread(this->socket);
	}

	TcpSocket::~TcpSocket()
	{
		this->thread->running = false;
		this->thread->join();
		delete this->thread;
	}

	bool TcpSocket::isConnecting()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == CONNECTING);
		this->thread->mutex.unlock();
		return result;
	}

	bool TcpSocket::isConnected()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == CONNECTED || this->thread->state == DISCONNECTING);
		this->thread->mutex.unlock();
		return result;
	}

	bool TcpSocket::isDisconnecting()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == DISCONNECTING);
		this->thread->mutex.unlock();
		return result;
	}

	void TcpSocket::update(float timeSinceLastFrame)
	{
		Socket::update(timeSinceLastFrame);
		this->thread->mutex.lock();
		State state = this->thread->state;
		WorkerThread::Result result = this->thread->result;
		Host host = this->thread->host;
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
				this->host = Host();
				this->port = 0;
				this->socketDelegate->onDisconnected(this, host, port);
				break;
			}
		}
		else if (result == WorkerThread::FAILED)
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

	bool TcpSocket::connect(Host host, unsigned short port)
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
	
	bool TcpSocket::disconnect()
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
			this->host = Host();
			this->port = 0;
		}
		return result;
	}

	int TcpSocket::send(hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
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
		return this->_send(stream, count);
	}

	int TcpSocket::send(chstr data)
	{
		return Socket::send(data);
	}

	int TcpSocket::receive(hstream* stream, int maxBytes)
	{
		if (!this->_canReceive(stream))
		{
			return false;
		}
		this->thread->mutex.lock();
		this->receiver->mutex.lock();
		State socketState = this->thread->state;
		State receiverState = this->receiver->state;
		this->receiver->mutex.unlock();
		this->thread->mutex.unlock();
		if (!this->_checkStartReceiveStatus(socketState, receiverState))
		{
			return 0;
		}
		return this->_receive(stream, maxBytes);
	}

	bool TcpSocket::connectAsync(Host host, unsigned short port)
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
	
	bool TcpSocket::disconnectAsync()
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

	bool TcpSocket::sendAsync(hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
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
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)count, stream->size() - stream->position()));
		this->sender->stream->rewind();
		this->sender->state = RUNNING;
		this->sender->mutex.unlock();
		this->thread->mutex.unlock();
		this->sender->start();
		return true;
	}

	bool TcpSocket::sendAsync(chstr data)
	{
		return Socket::sendAsync(data);
	}

	bool TcpSocket::startReceiveAsync(int maxBytes)
	{
		this->thread->mutex.lock();
		this->receiver->mutex.lock();
		State socketState = this->thread->state;
		State receiverState = this->receiver->state;
		if (!this->_checkStartReceiveStatus(socketState, receiverState))
		{
			this->receiver->mutex.unlock();
			this->thread->mutex.unlock();
			return false;
		}
		this->receiver->maxBytes = maxBytes;
		this->receiver->state = RUNNING;
		this->receiver->mutex.unlock();
		this->thread->mutex.unlock();
		this->receiver->start();
		return true;
	}

	void TcpSocket::_activateConnection(Host host, unsigned short port)
	{
		Base::_activateConnection(host, port);
		this->thread->state = CONNECTED;
	}

	bool TcpSocket::_checkConnectedStatus(State socketState, chstr action)
	{
		switch (socketState)
		{
		case IDLE:			hlog::warn(sakit::logTag, "Cannot " + action + ", not connected!");			return false;
		case CONNECTING:	hlog::warn(sakit::logTag, "Cannot " + action + ", still connecting!");		return false;
		case DISCONNECTING:	hlog::warn(sakit::logTag, "Cannot " + action + ", already disconnecting!");	return false;
		}
		return true;
	}

	bool TcpSocket::_checkConnectStatus(State socketState)
	{
		switch (socketState)
		{
		case CONNECTING:	hlog::warn(sakit::logTag, "Cannot connect, already connecting!");		return false;
		case CONNECTED:		hlog::warn(sakit::logTag, "Cannot connect, already connected!");		return false;
		case DISCONNECTING:	hlog::warn(sakit::logTag, "Cannot connect, already disconnecting!");	return false;
		}
		return true;
	}

	bool TcpSocket::_checkDisconnectStatus(State socketState, State senderState, State receiverState)
	{
		if (!this->_checkConnectedStatus(socketState, "disconnect"))
		{
			return false;
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

	bool TcpSocket::_checkSendStatus(State socketState, State senderState)
	{
		if (!this->_checkConnectedStatus(socketState, "send"))
		{
			return false;
		}
		return Socket::_checkSendStatus(senderState);
	}

	bool TcpSocket::_checkStartReceiveStatus(State socketState, State receiverState)
	{
		if (!this->_checkConnectedStatus(socketState, "start receiving"))
		{
			return false;
		}
		return Socket::_checkStartReceiveStatus(receiverState);
	}

}
