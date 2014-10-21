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
	
	void UdpServer::update(float timeDelta)
	{
		harray<Host> hosts;
		harray<unsigned short> ports;
		harray<hstream*> streams;
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThread(&this->udpServerThread->mutex);
		if (this->udpServerThread->streams.size() > 0)
		{
			hosts = this->udpServerThread->remoteHosts;
			ports = this->udpServerThread->remotePorts;
			streams = this->udpServerThread->streams;
			this->udpServerThread->remoteHosts.clear();
			this->udpServerThread->remotePorts.clear();
			this->udpServerThread->streams.clear();
		}
		lockThread.release();
		lock.release();
		for_iter (i, 0, streams.size())
		{
			this->udpServerDelegate->onReceived(this, hosts[i], ports[i], streams[i]);
			delete streams[i];
		}
		Server::update(timeDelta);
	}

	bool UdpServer::receive(hstream* stream, Host& host, unsigned short& port)
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canStart(this->state))
		{
			return false;
		}
		this->state = RUNNING;
		lock.release();
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
		lock.acquire(&this->mutexState);
		this->state = BOUND;
		return result;
	}

}
