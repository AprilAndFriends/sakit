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
#include "State.h"
#include "UdpServer.h"
#include "UdpServerDelegate.h"
#include "UdpServerThread.h"
#include "UdpSocket.h"

namespace sakit
{
	UdpServer::UdpServer(UdpServerDelegate* udpServerDelegate) : Server(dynamic_cast<ServerDelegate*>(udpServerDelegate))
	{
		this->udpServerDelegate = udpServerDelegate;
		this->serverThread = this->udpServerThread = new UdpServerThread(this->socket);
		this->socket->setConnectionLess(true);
		this->__register();
	}

	UdpServer::~UdpServer()
	{
		this->__unregister();
	}
	
	void UdpServer::update(float timeSinceLastFrame)
	{
		harray<Host> hosts;
		harray<unsigned short> ports;
		harray<hstream*> streams;
		this->mutexState.lock();
		this->udpServerThread->mutex.lock();
		if (this->udpServerThread->streams.size() > 0)
		{
			hosts = this->udpServerThread->remoteHosts;
			ports = this->udpServerThread->remotePorts;
			streams = this->udpServerThread->streams;
			this->udpServerThread->remoteHosts.clear();
			this->udpServerThread->remotePorts.clear();
			this->udpServerThread->streams.clear();
		}
		this->udpServerThread->mutex.unlock();
		this->mutexState.unlock();
		for_iter (i, 0, streams.size())
		{
			this->udpServerDelegate->onReceived(this, hosts[i], ports[i], streams[i]);
			delete streams[i];
		}
		Server::update(timeSinceLastFrame);
	}

	bool UdpServer::receive(hstream* stream, Host& host, unsigned short& port, float timeout)
	{
		this->mutexState.lock();
		if (!this->_canStart(this->state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->state = RUNNING;
		this->mutexState.unlock();
		float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
		timeout *= 1000.0f;
		bool result = false;
		while (true)
		{
			if (this->socket->receiveFrom(stream, host, port))
			{
				if (stream->size() > 0)
				{
					result = true;
				}
				break;
			}
			timeout -= retryTimeout;
			if (timeout <= 0.0f)
			{
				break;
			}
			hthread::sleep(retryTimeout);
		}
		this->mutexState.lock();
		this->state = BOUND;
		this->mutexState.unlock();
		return result;
	}

}
