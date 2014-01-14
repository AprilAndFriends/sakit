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
#include "ReceiverThread.h"
#include "sakit.h"
#include "SenderThread.h"
#include "SocketDelegate.h"
#include "UdpSocket.h"

namespace sakit
{
	UdpSocket::UdpSocket(SocketDelegate* socketDelegate) : Socket(socketDelegate), multicastGroup(false)
	{
		this->socket->setConnectionLess(true);
	}

	UdpSocket::~UdpSocket()
	{
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
		bool result = this->socket->createSocket(host, port);
		if (result)
		{
			this->host = host;
			this->port = port;
		}
		return result;
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

	int UdpSocket::send(hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		this->sender->mutex.lock();
		State senderState = this->sender->state;
		this->sender->mutex.unlock();
		if (!this->_checkSendStatus(senderState))
		{
			return false;
		}
		return this->_send(stream, count);
	}

	int UdpSocket::send(chstr data)
	{
		return Socket::send(data);
	}

	int UdpSocket::receive(hstream* stream, int maxBytes)
	{
		if (!this->_canReceive(stream))
		{
			return false;
		}
		this->receiver->mutex.lock();
		State receiverState = this->receiver->state;
		this->receiver->mutex.unlock();
		if (!this->_checkStartReceiveStatus(receiverState))
		{
			return 0;
		}
		return this->_receive(stream, maxBytes);
	}

	bool UdpSocket::sendAsync(hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		this->sender->mutex.lock();
		State senderState = this->sender->state;
		if (!this->_checkSendStatus(senderState))
		{
			this->sender->mutex.unlock();
			return false;
		}
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)count, stream->size() - stream->position()));
		this->sender->stream->rewind();
		this->sender->state = RUNNING;
		this->sender->mutex.unlock();
		this->sender->start();
		return true;
	}

	bool UdpSocket::sendAsync(chstr data)
	{
		return Socket::sendAsync(data);
	}

	bool UdpSocket::startReceiveAsync(int maxBytes)
	{
		this->receiver->mutex.lock();
		State receiverState = this->receiver->state;
		if (!this->_checkStartReceiveStatus(receiverState))
		{
			this->receiver->mutex.unlock();
			return false;
		}
		this->receiver->maxBytes = maxBytes;
		this->receiver->state = RUNNING;
		this->receiver->mutex.unlock();
		this->receiver->start();
		return true;
	}

	void UdpSocket::_activateConnection(Host host, unsigned short port)
	{
		Base::_activateConnection(host, port);
		this->setDestination(host, port);
	}

	bool UdpSocket::broadcast(unsigned short port, hstream* stream, int count )
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
		return PlatformSocket::broadcast(PlatformSocket::getNetworkAdapters(), port, &stream);
	}

	bool UdpSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return PlatformSocket::broadcast(adapters, port, &stream);
	}

}
