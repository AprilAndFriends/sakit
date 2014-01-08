/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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

	TcpServer::TcpServer(TcpServerDelegate* serverDelegate, SocketDelegate* socketDelegate) :
		Server(serverDelegate, socketDelegate)
	{
		this->basicThread = this->thread = new TcpServerThread(this->socket, this->acceptedDelegate);
		this->basicDelegate = this->serverDelegate = serverDelegate;
	}

	TcpServer::~TcpServer()
	{
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

	void TcpServer::update(float timeSinceLastFrame)
	{
		foreach (TcpSocket*, it, this->sockets)
		{
			(*it)->update(timeSinceLastFrame);
		}
		this->_updateSockets();
		this->thread->mutex.lock();
		State state = this->thread->state;
		WorkerThread::Result result = this->thread->result;
		Ip host = this->thread->host;
		unsigned short port = this->thread->port;
		if (this->thread->sockets.size() > 0)
		{
			harray<TcpSocket*> sockets = this->thread->sockets;
			this->thread->sockets.clear();
			this->thread->mutex.unlock();
			this->sockets += sockets;
			foreach (TcpSocket*, it, sockets)
			{
				this->serverDelegate->onAccepted(this, (*it));
			}
		}
		else
		{
			this->thread->mutex.unlock();
		}
		Server::update(timeSinceLastFrame);
	}

	TcpSocket* TcpServer::accept(float timeout)
	{
		TcpSocket* socket = NULL;
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		if (this->_checkStartStatus(state))
		{
			float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
			timeout *= 1000.0f;
			while (true)
			{
				if (!this->socket->listen())
				{
					break;
				}
				socket = new TcpSocket(this->acceptedDelegate);
				connectionsMutex.lock();
				connections -= socket;
				connectionsMutex.unlock();
				if (this->socket->accept(socket))
				{
					this->sockets += socket;
					break;
				}
				delete socket;
				socket = NULL;
				timeout -= retryTimeout;
				if (timeout <= 0.0f)
				{
					break;
				}
				hthread::sleep(retryTimeout);
			}
		}
		return socket;
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
