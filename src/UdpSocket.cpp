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
	UdpSocket::UdpSocket(UdpSocketDelegate* socketDelegate) : Socket(dynamic_cast<SocketDelegate*>(socketDelegate), BOUND),
		Binder(this->socket, dynamic_cast<BinderDelegate*>(socketDelegate))
	{
		this->udpSocketDelegate = socketDelegate;
		this->socket->setConnectionLess(true);
		this->receiver = this->udpReceiver = new UdpReceiverThread(this->socket);
		Binder::_integrate(&this->state, &this->mutexState, &this->localHost, &this->localPort);
		this->__register();
	}

	UdpSocket::~UdpSocket()
	{
		this->_clear();
		this->__unregister();
	}

	bool UdpSocket::hasDestination()
	{
		return this->socket->isConnected();
	}

	bool UdpSocket::setDestination(Host remoteHost, unsigned short remotePort)
	{
		if (!this->isBound() && !this->bind())
		{
			return false;
		}
		// this is not a real connect on UDP, it just does its job of setting a proper remote host
		if (!this->socket->connect(remoteHost, remotePort, this->localHost, this->localPort))
		{
			this->_clear();
			return false;
		}
		this->remoteHost = remoteHost;
		this->remotePort = remotePort;
		return true;
	}

	void UdpSocket::_clear()
	{
		this->remoteHost = Host();
		this->remotePort = 0;
		this->localHost = Host();
		this->localPort = 0;
		this->multicastHosts.clear();
		this->socket->disconnect();
	}

	bool UdpSocket::joinMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		if (!this->isBound() && !this->bind())
		{
			this->_clear();
			return false;
		}
		if (!this->socket->joinMulticastGroup(interfaceHost, groupAddress))
		{
			return false;
		}
		this->multicastHosts += std::pair<Host, Host>(interfaceHost, groupAddress);
		return true;
	}

	bool UdpSocket::leaveMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		std::pair<Host, Host> pair(interfaceHost, groupAddress);
		if (!this->multicastHosts.contains(pair))
		{
			hlog::warnf(sakit::logTag, "Cannot leave multicast group, interface %s not in group %s!", interfaceHost.toString().c_str(), groupAddress.toString().c_str());
			return false;
		}
		if (!this->socket->leaveMulticastGroup(interfaceHost, groupAddress))
		{
			return false;
		}
		this->multicastHosts -= pair;
		return true;
	}

	bool UdpSocket::setMulticastInterface(Host interfaceHost)
	{
		return this->socket->setMulticastInterface(interfaceHost);
	}

	bool UdpSocket::setMulticastTtl(int value)
	{
		return this->socket->setMulticastTtl(value);
	}

	bool UdpSocket::setMulticastLoopback(bool value)
	{
		return this->socket->setMulticastLoopback(value);
	}

	void UdpSocket::update(float timeSinceLastFrame)
	{
		Binder::_update(timeSinceLastFrame);
		Socket::update(timeSinceLastFrame);
	}

	void UdpSocket::_updateReceiving()
	{
		harray<Host> remoteHosts;
		harray<unsigned short> remotePorts;
		harray<hstream*> streams;
		this->mutexState.lock();
		this->receiver->mutex.lock();
		if (this->udpReceiver->streams.size() > 0)
		{
			remoteHosts = this->udpReceiver->remoteHosts;
			remotePorts = this->udpReceiver->remotePorts;
			streams = this->udpReceiver->streams;
			this->udpReceiver->remoteHosts.clear();
			this->udpReceiver->remotePorts.clear();
			this->udpReceiver->streams.clear();
		}
		State result = this->receiver->result;
		if (result == RUNNING || result == IDLE)
		{
			this->receiver->mutex.unlock();
			this->mutexState.unlock();
			for_iter (i, 0, streams.size())
			{
				this->udpSocketDelegate->onReceived(this, remoteHosts[i], remotePorts[i], streams[i]);
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
			this->udpSocketDelegate->onReceived(this, remoteHosts[i], remotePorts[i], streams[i]);
			delete streams[i];
		}
		// delegate calls
		if (result == FINISHED)
		{
			this->socketDelegate->onReceiveFinished(this);
		}
	}

	int UdpSocket::receive(hstream* stream, Host& remoteHost, unsigned short& remotePort)
	{
		if (!this->_prepareReceive(stream))
		{
			return 0;
		}
		return this->_finishReceive(this->_receiveFromDirect(stream, remoteHost, remotePort));
	}

	bool UdpSocket::startReceiveAsync(int maxPackages)
	{
		return this->_startReceiveAsync(maxPackages);
	}

	void UdpSocket::_activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort)
	{
		SocketBase::_activateConnection(remoteHost, remotePort, localHost, localPort);
		this->socket->setRemoteAddress(remoteHost, remotePort);
	}

	bool UdpSocket::broadcast(unsigned short remotePort, hstream* stream, int count)
	{
		return PlatformSocket::broadcast(PlatformSocket::getNetworkAdapters(), remotePort, stream, count);
	}

	bool UdpSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short remotePort, hstream* stream, int count)
	{
		return PlatformSocket::broadcast(adapters, remotePort, stream, count);
	}

	bool UdpSocket::broadcast(unsigned short remotePort, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return PlatformSocket::broadcast(PlatformSocket::getNetworkAdapters(), remotePort, &stream, stream.size());
	}

	bool UdpSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short remotePort, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return PlatformSocket::broadcast(adapters, remotePort, &stream, stream.size());
	}

}
