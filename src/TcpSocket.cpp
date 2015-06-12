/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "ConnectorThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "sakitUtil.h"
#include "SenderThread.h"
#include "TcpReceiverThread.h"
#include "TcpSocket.h"
#include "TcpSocketDelegate.h"

namespace sakit
{
	TcpSocket::TcpSocket(TcpSocketDelegate* socketDelegate) : Socket(dynamic_cast<SocketDelegate*>(socketDelegate), CONNECTED),
		Connector(this->socket, dynamic_cast<ConnectorDelegate*>(socketDelegate))
	{
		this->tcpSocketDelegate = socketDelegate;
		this->socket->setConnectionLess(false);
		this->receiver = this->tcpReceiver = new TcpReceiverThread(this->socket, &this->timeout, &this->retryFrequency);
		Connector::_integrate(&this->state, &this->mutexState, &this->remoteHost, &this->remotePort, &this->localHost, &this->localPort, &this->timeout, &this->retryFrequency);
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

	void TcpSocket::update(float timeDelta)
	{
		Socket::update(timeDelta);
		Connector::_update(timeDelta);
	}

	void TcpSocket::_updateReceiving()
	{
		hstream* stream = NULL;
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->receiver->mutex);
		if (this->tcpReceiver->stream->size() > 0)
		{
			stream = this->tcpReceiver->stream;
			this->tcpReceiver->stream = new hstream();
		}
		State result = this->receiver->result;
		if (result == RUNNING || result == IDLE)
		{
			if (stream != NULL)
			{
				lockThread.release();
				lock.release();
				stream->rewind();
				this->tcpSocketDelegate->onReceived(this, stream);
				delete stream;
			}
			return;
		}
		this->receiver->result = IDLE;
		this->state = (this->state == SENDING_RECEIVING ? SENDING : this->idleState);
		lockThread.release();
		lock.release();
		if (stream != NULL)
		{
			stream->rewind();
			this->tcpSocketDelegate->onReceived(this, stream);
			delete stream;
		}
		// delegate calls
		switch (result)
		{
		case FINISHED:	this->socketDelegate->onReceiveFinished(this);	break;
		case FAILED:	this->tcpSocketDelegate->onReceiveFailed(this);	break;
		default:														break;
		}
	}

	int TcpSocket::receive(hstream* stream, int maxBytes)
	{
		if (!this->_prepareReceive(stream))
		{
			return 0;
		}
		return this->_finishReceive(this->_receiveDirect(stream, maxBytes));
	}
	
	hstr TcpSocket::receive(int maxBytes)
	{
		hstream stream;
		int size = this->receive(&stream, maxBytes);
		unsigned char terminator = 0;
		stream.writeRaw(&terminator, 1); // Terminator 2 was better though
		return hstr((char*)stream, size);
	}

	bool TcpSocket::startReceiveAsync(int maxBytes)
	{
		return this->_startReceiveAsync(maxBytes);
	}

	void TcpSocket::_activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort)
	{
		SocketBase::_activateConnection(remoteHost, remotePort, localHost, localPort);
		hmutex::ScopeLock lock(&this->mutexState);
		this->state = CONNECTED;
	}

}
