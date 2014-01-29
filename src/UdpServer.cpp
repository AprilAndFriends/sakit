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
	UdpServer::UdpServer(UdpServerDelegate* serverDelegate) : Server(serverDelegate)
	{
		this->basicDelegate = this->serverDelegate = serverDelegate;
		this->basicThread = this->thread = new UdpServerThread(this->socket);
		this->socket->setConnectionLess(true);
		this->__register();
	}

	UdpServer::~UdpServer()
	{
		this->__unregister();
	}
	
	void UdpServer::update(float timeSinceLastFrame)
	{
		this->thread->mutex.lock();
		if (this->thread->streams.size() > 0)
		{
			harray<Host> hosts = this->thread->hosts;
			harray<unsigned short> ports = this->thread->ports;
			harray<hstream*> streams = this->thread->streams;
			this->thread->hosts.clear();
			this->thread->ports.clear();
			this->thread->streams.clear();
			this->thread->mutex.unlock();
			for_iter (i, 0, streams.size())
			{
				this->serverDelegate->onReceived(this, hosts[i], ports[i], streams[i]);
			}
		}
		else
		{
			this->thread->mutex.unlock();
		}
		Server::update(timeSinceLastFrame);
	}

	bool UdpServer::receive(hstream* stream, Host& host, unsigned short& port, float timeout)
	{
		bool result = false;
		this->thread->mutex.lock();
		State state = this->thread->_state;
		this->thread->mutex.unlock();
		if (this->_canStart(state))
		{
			float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
			timeout *= 1000.0f;
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
		}
		return result;
	}

}
