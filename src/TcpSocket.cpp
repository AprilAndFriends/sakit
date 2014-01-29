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
#include "sakit.h"
#include "SenderThread.h"
#include "TcpReceiverThread.h"
#include "TcpSocket.h"
#include "TcpSocketDelegate.h"

namespace sakit
{
	TcpSocket::TcpSocket(TcpSocketDelegate* socketDelegate) : Socket(socketDelegate), Connector(this->socket, dynamic_cast<ConnectorDelegate*>(socketDelegate))
	{
		this->tcpSocketDelegate = socketDelegate;
		this->socket->setConnectionLess(false);
		this->receiver = this->tcpReceiver = new TcpReceiverThread(this->socket);
		Connector::_integrate(&this->state, &this->mutexState, &this->host, &this->port);
		this->__register();
	}

	TcpSocket::~TcpSocket()
	{
		this->__unregister();
	}

	bool TcpSocket::setNagleAlgorithmActive(bool value)
	{
		return this->socket->setNagleAlgorithmActive(value);
	}

	void TcpSocket::update(float timeSinceLastFrame)
	{
		Socket::update(timeSinceLastFrame);
		Connector::_update(timeSinceLastFrame);
	}

	void TcpSocket::_updateReceiving()
	{
		this->receiver->mutex.lock();
		State result = this->receiver->result;
		if (this->tcpReceiver->stream->size() > 0)
		{
			hstream* stream = this->tcpReceiver->stream;
			this->tcpReceiver->stream = new hstream();
			this->receiver->mutex.unlock();
			stream->rewind();
			this->tcpSocketDelegate->onReceived(this, stream);
			delete stream;
		}
		else
		{
			this->receiver->mutex.unlock();
		}
		if (result == RUNNING || result == IDLE)
		{
			return;
		}
		this->receiver->mutex.lock();
		this->receiver->result = IDLE;
		this->receiver->state = IDLE;
		this->receiver->mutex.unlock();
		if (result == FINISHED)
		{
			this->socketDelegate->onReceiveFinished(this);
		}
		else if (result == FAILED)
		{
			this->tcpSocketDelegate->onReceiveFailed(this);
		}
	}

	int TcpSocket::_send(hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		//this->thread->mutex.lock();
		this->sender->mutex.lock();
		//State socketState = this->thread->state;
		State senderState = this->sender->state;
		this->sender->mutex.unlock();
		//this->thread->mutex.unlock();
		if (!this->_checkSendStatus(IDLE, senderState))
		{
			return false;
		}
		return this->_sendDirect(stream, count);
	}

	int TcpSocket::receive(hstream* stream, int maxBytes)
	{
		if (!this->_canReceive(stream))
		{
			return false;
		}
		//this->thread->mutex.lock();
		this->receiver->mutex.lock();
		//State socketState = this->thread->state;
		State receiverState = this->receiver->state;
		this->receiver->mutex.unlock();
		//this->thread->mutex.unlock();
		if (!this->_checkStartReceiveStatus(IDLE, receiverState))
		{
			return 0;
		}
		return this->_receiveDirect(stream, maxBytes);
	}

	bool TcpSocket::_sendAsync(hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		//this->thread->mutex.lock();
		this->sender->mutex.lock();
		//State socketState = this->thread->state;
		State senderState = this->sender->state;
		if (!this->_checkSendStatus(IDLE, senderState))
		{
			this->sender->mutex.unlock();
			//this->thread->mutex.unlock();
			return false;
		}
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)count, stream->size() - stream->position()));
		this->sender->stream->rewind();
		this->sender->state = RUNNING;
		this->sender->mutex.unlock();
		//this->thread->mutex.unlock();
		this->sender->start();
		return true;
	}

	bool TcpSocket::startReceiveAsync(int maxBytes)
	{
		//this->thread->mutex.lock();
		this->receiver->mutex.lock();
		//State socketState = this->thread->state;
		State receiverState = this->receiver->state;
		if (!this->_checkStartReceiveStatus(IDLE, receiverState))
		{
			this->receiver->mutex.unlock();
			//this->thread->mutex.unlock();
			return false;
		}
		this->tcpReceiver->maxBytes = maxBytes;
		this->receiver->state = RUNNING;
		this->receiver->mutex.unlock();
		//this->thread->mutex.unlock();
		this->receiver->start();
		return true;
	}

	void TcpSocket::_activateConnection(Host host, unsigned short port)
	{
		Base::_activateConnection(host, port);
		this->mutexState.lock();
		this->state = CONNECTED;
		this->mutexState.unlock();
	}

	bool TcpSocket::_checkSendStatus(State socketState, State senderState)
	{
		/*
		if (!this->_checkConnectedStatus(socketState, "send"))
		{
			return false;
		}
		*/
		return Socket::_checkSendStatus(senderState);
	}

	bool TcpSocket::_checkStartReceiveStatus(State socketState, State receiverState)
	{
		/*
		if (!this->_checkConnectedStatus(socketState, "start receiving"))
		{
			return false;
		}
		*/
		return Socket::_checkStartReceiveStatus(receiverState);
	}

}
