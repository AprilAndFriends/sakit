/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hmutex.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "TcpServer.h"
#include "TcpServerDelegate.h"
#include "TcpServerThread.h"
#include "TcpSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;
	extern hmutex updateMutex;

	TcpServer::TcpServer(TcpServerDelegate* tcpServerDelegate, TcpSocketDelegate* acceptedDelegate) :
		Server(dynamic_cast<ServerDelegate*>(tcpServerDelegate))
	{
		this->serverThread = this->tcpServerThread = new TcpServerThread(this->socket, this->acceptedDelegate, &this->timeout, &this->retryFrequency);
		this->tcpServerDelegate = tcpServerDelegate;
		this->acceptedDelegate = acceptedDelegate;
		this->socket->setConnectionLess(false);
		this->__register();
	}

	TcpServer::~TcpServer()
	{
		this->__unregister();
		foreach (TcpSocket*, it, this->sockets)
		{
			delete (*it);
		}
	}
	
	harray<TcpSocket*> TcpServer::getSockets()
	{
		this->_updateSockets();
		return this->sockets;
	}

	void TcpServer::update(float timeDelta)
	{
		foreach (TcpSocket*, it, this->sockets)
		{
			(*it)->update(timeDelta);
		}
		this->_updateSockets();
		harray<TcpSocket*> sockets;
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThreadResult(&this->tcpServerThread->resultMutex);
		hmutex::ScopeLock lockThreadSockets(&this->tcpServerThread->socketsMutex);
		if (this->tcpServerThread->sockets.size() > 0)
		{
			sockets = this->tcpServerThread->sockets;
			this->tcpServerThread->sockets.clear();
			this->sockets += sockets;
		}
		lockThreadSockets.release();
		lockThreadResult.release();
		lock.release();
		foreach (TcpSocket*, it, sockets)
		{
			this->tcpServerDelegate->onAccepted(this, (*it));
		}
		Server::update(timeDelta);
	}

	TcpSocket* TcpServer::accept()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canStart(this->state))
		{
			return NULL;
		}
		this->state = State::Running;
		lock.release();
		TcpSocket* tcpSocket = new TcpSocket(this->acceptedDelegate);
		hmutex::ScopeLock lockUpdate(&updateMutex);
		lock.acquire(&connectionsMutex);
		connections -= tcpSocket;
		lock.release();
		lockUpdate.release();
		float time = 0.0f;
		while (true)
		{
			if (!this->socket->listen())
			{
				delete tcpSocket;
				tcpSocket = NULL;
				break;
			}
			if (this->socket->accept(tcpSocket))
			{
				this->sockets += tcpSocket;
				break;
			}
			time += this->retryFrequency;
			if (time >= this->timeout)
			{
				delete tcpSocket;
				tcpSocket = NULL;
				break;
			}
			hthread::sleep(this->retryFrequency * 1000.0f);
		}
		lock.acquire(&this->mutexState);
		this->state = State::Bound;
		return tcpSocket;
	}

	void TcpServer::_updateSockets()
	{
		harray<TcpSocket*> sockets = this->sockets;
		this->sockets.clear();
		foreach (TcpSocket*, it, sockets)
		{
			if ((*it)->isConnected())
			{
				this->sockets += (*it);
			}
			else
			{
				delete (*it);
			}
		}
	}
	
}
