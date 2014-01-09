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
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SenderThread.h"
#include "UdpServer.h"
#include "UdpServerDelegate.h"
#include "UdpServerThread.h"
#include "UdpSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;

	UdpServer::UdpServer(UdpServerDelegate* serverDelegate, SocketDelegate* receivedDelegate) : Server(serverDelegate)
	{
		this->basicDelegate = this->serverDelegate = serverDelegate;
		this->receivedDelegate = receivedDelegate;
		this->basicThread = this->thread = new UdpServerThread(this->socket, this->receivedDelegate);
		this->socket->setConnectionLess(true);
	}

	UdpServer::~UdpServer()
	{
	}
	
	harray<UdpSocket*> UdpServer::getSockets()
	{
		this->_updateSockets();
		return this->sockets;
	}

	void UdpServer::update(float timeSinceLastFrame)
	{
		foreach (UdpSocket*, it, this->sockets)
		{
			(*it)->update(timeSinceLastFrame);
		}
		this->_updateSockets();
		this->thread->mutex.lock();
		if (this->thread->streams.size() > 0)
		{
			harray<UdpSocket*> sockets = this->thread->sockets;
			harray<hstream*> streams = this->thread->streams;
			this->thread->sockets.clear();
			this->thread->streams.clear();
			this->thread->mutex.unlock();
			this->sockets += sockets;
			for_iter (i, 0, sockets.size())
			{
				this->serverDelegate->onReceived(this, sockets[i], streams[i]);
				delete streams[i];
			}
		}
		else
		{
			this->thread->mutex.unlock();
		}
		Server::update(timeSinceLastFrame);
	}

	void UdpServer::_updateSockets()
	{
		harray<UdpSocket*> sockets = this->sockets;
		this->sockets.clear();
		foreach (UdpSocket*, it, sockets)
		{
			if ((*it)->hasDestination())
			{
				this->sockets += (*it);
			}
			else
			{
				delete (*it);
			}
		}
	}
	
	UdpSocket* UdpServer::receive(hstream* stream, float timeout)
	{
		UdpSocket* socket = NULL;
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		if (this->_checkStartStatus(state))
		{
			float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
			timeout *= 1000.0f;
			while (true)
			{
				socket = new UdpSocket(this->receivedDelegate);
				connectionsMutex.lock();
				connections -= socket;
				connectionsMutex.unlock();
				if (!this->socket->receiveFrom(stream, socket))
				{
					delete socket;
					socket = NULL;
					break;
				}
				if (stream->size() > 0)
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

}
