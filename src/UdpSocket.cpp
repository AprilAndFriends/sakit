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
	UdpSocket::UdpSocket(UdpSocketDelegate* socketDelegate) : Socket(dynamic_cast<SocketDelegate*>(socketDelegate), BOUND), multicastGroup(false)
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
		this->receiver->mutex.lock();
		State result = this->receiver->result;
		if (this->udpReceiver->streams.size() > 0)
		{
			harray<Host> hosts = this->udpReceiver->hosts;
			harray<unsigned short> ports = this->udpReceiver->ports;
			harray<hstream*> streams = this->udpReceiver->streams;
			this->udpReceiver->hosts.clear();
			this->udpReceiver->ports.clear();
			this->udpReceiver->streams.clear();
			this->receiver->mutex.unlock();
			for_iter (i, 0, streams.size())
			{
				this->udpSocketDelegate->onReceived(this, hosts[i], ports[i], streams[i]);
			}
		}
		else
		{
			this->receiver->mutex.unlock();
		}
		if (result == RUNNING || result == IDLE)
		{
			return;
		}
		this->receiver->mutex.lock();
		this->receiver->result = IDLE;
		this->receiver->_state = IDLE;
		this->receiver->mutex.unlock();
		if (result == FINISHED)
		{
			this->socketDelegate->onReceiveFinished(this);
		}
	}

	int UdpSocket::receive(hstream* stream, Host& host, unsigned short& port)
	{
		if (!this->_checkReceiveParameters(stream))
		{
			return false;
		}
		this->receiver->mutex.lock();
		State receiverState = this->receiver->_state;
		this->receiver->mutex.unlock();
		if (!this->_checkStartReceiveStatus(receiverState))
		{
			return 0;
		}
		return this->_receiveFromDirect(stream, host, port);
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
