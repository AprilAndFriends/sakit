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

	UdpServer::UdpServer(UdpServerDelegate* serverDelegate, SocketDelegate* senderDelegate) : Server(serverDelegate)
	{
		this->basicThread = this->thread = new UdpServerThread(this->socket);
		this->basicDelegate = this->serverDelegate = serverDelegate;
		this->senderDelegate = senderDelegate;
		this->socket->setConnectionLess(true);
	}

	UdpServer::~UdpServer()
	{
	}
	
	harray<UdpSocket*> UdpServer::getSenderSockets()
	{
		this->_updateSockets();
		return this->senderSockets;
	}

	void UdpServer::update(float timeSinceLastFrame)
	{
		foreach (UdpSocket*, it, this->senderSockets)
		{
			(*it)->update(timeSinceLastFrame);
		}
		this->_updateSockets();
		this->thread->mutex.lock();
		if (this->thread->streams.size() > 0)
		{
			harray<hstream*> streams = this->thread->streams;
			harray<Ip> hosts = this->thread->hosts;
			harray<unsigned short> ports = this->thread->ports;
			this->thread->streams.clear();
			this->thread->hosts.clear();
			this->thread->ports.clear();
			this->thread->mutex.unlock();
			for_iter (i, 0, streams.size())
			{
				this->serverDelegate->onReceived(this, streams[i], hosts[i], ports[i]);
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
		harray<UdpSocket*> sockets = this->senderSockets;
		this->senderSockets.clear();
		foreach (UdpSocket*, it, sockets)
		{
			if ((*it)->isSending())
			{
				this->senderSockets += (*it);
			}
			else
			{
				delete (*it);
			}
		}
	}
	
	int UdpServer::send(Ip host, unsigned short port, hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		if (!this->_checkSendStatus(host, port))
		{
			return false;
		}
		int sent = 0;
		UdpSocket* socket = new UdpSocket(this->senderDelegate);
		if (socket->setDestination(host, port))
		{
			sent = socket->send(stream, count);
		}
		delete socket;
		return sent;
	}

	int UdpServer::send(Ip host, unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->send(host, port, &stream);
	}

	bool UdpServer::receive(hstream* stream, Ip* host, unsigned short* port)
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		return (this->_checkStartStatus(state) && this->socket->receiveFrom(stream, host, port));
	}

	bool UdpServer::sendAsync(Ip host, unsigned short port, hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		if (!this->_checkSendStatus(host, port))
		{
			return false;
		}
		UdpSocket* socket = new UdpSocket(this->senderDelegate);
		connectionsMutex.lock();
		connections -= socket;
		connectionsMutex.unlock();
		bool result = (socket->setDestination(host, port) && socket->sendAsync(stream, count));
		if (result)
		{
			this->senderSockets += socket;
		}
		else
		{
			delete socket;
		}
		return result;
	}

	bool UdpServer::sendAsync(Ip host, unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->sendAsync(host, port, &stream);
	}

	bool UdpServer::_checkSendStatus(Ip host, unsigned short port)
	{
		foreach (UdpSocket*, it, this->senderSockets)
		{
			if ((*it)->hasDestination() && (*it)->getHost() == host && (*it)->getPort() == port)
			{
				hlog::warn(sakit::logTag, "Cannot send, already sending!");
				return false;
			}
		}
		return true;
	}

}
