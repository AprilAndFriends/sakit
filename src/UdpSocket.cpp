/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SenderThread.h"
#include "State.h"
#include "UdpReceiverThread.h"
#include "UdpSocket.h"
#include "UdpSocketDelegate.h"

namespace sakit
{
	UdpSocket::UdpSocket(UdpSocketDelegate* socketDelegate) : Socket(dynamic_cast<SocketDelegate*>(socketDelegate), IDLE), multicastGroup(false)
	{
		this->udpSocketDelegate = socketDelegate;
		this->socket->setConnectionLess(true);
		this->receiver = this->udpReceiver = new UdpReceiverThread(this->socket);
		this->__register();
	}

	UdpSocket::~UdpSocket()
	{
		this->__unregister();
	}

	bool UdpSocket::hasDestination()
	{
		return this->socket->isConnected();
	}

	bool UdpSocket::setDestination(Host host, unsigned short port)
	{
		if (this->hasDestination())
		{
			this->clearDestination();
		}
#ifndef _WINRT
		if (!this->socket->createSocket(host, port))
#else // WinRT is a special kid and UDP requires a "connection"
		if (!this->socket->connect(host, port))
#endif
		{
			this->clearDestination();
			return false;
		}
		this->host = host;
		this->port = port;
		return true;
	}

	bool UdpSocket::clearDestination()
	{
		this->host = Host();
		this->port = 0;
		this->multicastGroup = false;
		return this->socket->disconnect();
	}

	bool UdpSocket::joinMulticastGroup(Host address, unsigned short port, Host groupAddress)
	{
		if (this->hasDestination())
		{
			this->clearDestination();
		}
		bool result = this->socket->joinMulticastGroup(address, port, groupAddress);
		if (result)
		{
			this->host = host;
			this->port = port;
			this->multicastGroup = true;
		}
		return result;
	}

	bool UdpSocket::setMulticastInterface(Host address)
	{
		return this->socket->setMulticastInterface(address);
	}

	bool UdpSocket::setMulticastTtl(int value)
	{
		return this->socket->setMulticastTtl(value);
	}

	bool UdpSocket::setMulticastLoopback(bool value)
	{
		return this->socket->setMulticastTtl(value);
	}

	void UdpSocket::_updateReceiving()
	{
		harray<Host> hosts;
		harray<unsigned short> ports;
		harray<hstream*> streams;
		this->mutexState.lock();
		this->receiver->mutex.lock();
		if (this->udpReceiver->streams.size() > 0)
		{
			hosts = this->udpReceiver->hosts;
			ports = this->udpReceiver->ports;
			streams = this->udpReceiver->streams;
			this->udpReceiver->hosts.clear();
			this->udpReceiver->ports.clear();
			this->udpReceiver->streams.clear();
		}
		State state = this->state;
		State result = this->receiver->result;
		if (result == RUNNING || result == IDLE)
		{
			this->receiver->mutex.unlock();
			this->mutexState.unlock();
			for_iter (i, 0, streams.size())
			{
				this->udpSocketDelegate->onReceived(this, hosts[i], ports[i], streams[i]);
				delete streams[i];
			}
			return;
		}
		this->receiver->result = IDLE;
		this->state = (this->state == SENDING_RECEIVING ? SENDING : this->idleState);
		this->receiver->mutex.unlock();
		this->mutexState.unlock();
		for_iter (i, 0, streams.size())
		{
			this->udpSocketDelegate->onReceived(this, hosts[i], ports[i], streams[i]);
			delete streams[i];
		}
		// delegate calls
		if (result == FINISHED)
		{
			this->socketDelegate->onReceiveFinished(this);
		}
	}

	int UdpSocket::receive(hstream* stream, Host& host, unsigned short& port)
	{
		if (!this->_checkReceiveParameters(stream))
		{
			return 0;
		}
		this->mutexState.lock();
		State state = this->state;
		if (!this->_canReceive(state))
		{
			this->mutexState.unlock();
			return 0;
		}
		this->state = (this->state == SENDING ? SENDING_RECEIVING : RECEIVING);
		this->mutexState.unlock();
		int result = this->_receiveFromDirect(stream, host, port);
		this->mutexState.lock();
		this->state = (this->state == SENDING_RECEIVING ? SENDING : this->idleState);
		this->mutexState.unlock();
		return result;
	}

	bool UdpSocket::startReceiveAsync(int maxPackages)
	{
		return this->_startReceiveAsync(maxPackages);
	}

	void UdpSocket::_activateConnection(Host host, unsigned short port)
	{
		Base::_activateConnection(host, port);
		this->setDestination(host, port);
	}

	bool UdpSocket::broadcast(unsigned short port, hstream* stream, int count)
	{
		return PlatformSocket::broadcast(PlatformSocket::getNetworkAdapters(), port, stream, count);
	}

	bool UdpSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count)
	{
		return PlatformSocket::broadcast(adapters, port, stream, count);
	}

	bool UdpSocket::broadcast(unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return PlatformSocket::broadcast(PlatformSocket::getNetworkAdapters(), port, &stream, INT_MAX);
	}

	bool UdpSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return PlatformSocket::broadcast(adapters, port, &stream, INT_MAX);
	}

}
