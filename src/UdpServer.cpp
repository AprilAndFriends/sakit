/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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
		this->socket->setConnectionLess(true);
		this->udpServerDelegate = udpServerDelegate;
		this->serverThread = this->udpServerThread = new UdpServerThread(this->socket, &this->timeout, &this->retryFrequency);
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

	bool UdpServer::receive(hstream* stream, Host& host, unsigned short& port)
	{
		this->mutexState.lock();
		if (!this->_canStart(this->state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->state = RUNNING;
		this->mutexState.unlock();
		float time = 0.0f;
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
			time += this->retryFrequency;
			if (time >= this->timeout)
			{
				break;
			}
			hthread::sleep(this->retryFrequency * 1000.0f);
		}
		this->mutexState.lock();
		this->state = BOUND;
		this->mutexState.unlock();
		return result;
	}

}
