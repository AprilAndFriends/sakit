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
		this->receiver = this->tcpReceiver = new TcpReceiverThread(this->socket);
		Connector::_integrate(&this->state, &this->mutexState, &this->remoteHost, &this->remotePort, &this->localHost, &this->localPort);
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
		hstream* stream = NULL;
		this->mutexState.lock();
		this->receiver->mutex.lock();
		if (this->tcpReceiver->stream->size() > 0)
		{
			stream = this->tcpReceiver->stream;
			this->tcpReceiver->stream = new hstream();
		}
		State result = this->receiver->result;
		if (result == RUNNING || result == IDLE)
		{
			this->receiver->mutex.unlock();
			this->mutexState.unlock();
			if (stream != NULL)
			{
				stream->rewind();
				this->tcpSocketDelegate->onReceived(this, stream);
				delete stream;
			}
			return;
		}
		this->receiver->result = IDLE;
		this->state = (this->state == SENDING_RECEIVING ? SENDING : this->idleState);
		this->receiver->mutex.unlock();
		this->mutexState.unlock();
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
		stream.rewind();
		char* p = new char[size + 1];
		stream.read_raw(p, size);
		p[size] = 0;
		hstr result = p;
		delete [] p;
		
        return result;
    }

	bool TcpSocket::startReceiveAsync(int maxBytes)
	{
		return this->_startReceiveAsync(maxBytes);
	}

	void TcpSocket::_activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort)
	{
		SocketBase::_activateConnection(remoteHost, remotePort, localHost, localPort);
		this->mutexState.lock();
		this->state = CONNECTED;
		this->mutexState.unlock();
	}

}
