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
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "Connector.h"
#include "ConnectorDelegate.h"
#include "ConnectorThread.h"
#include "PlatformSocket.h"
#include "sakitUtil.h"

namespace sakit
{
	Connector::Connector(PlatformSocket* socket, ConnectorDelegate* connectorDelegate)
	{
		this->_socket = socket;
		this->_connectorDelegate = connectorDelegate;
		this->_thread = new ConnectorThread(this->_socket);
		this->_state = NULL;
		this->_mutexState = NULL;
		this->_remoteHost = NULL;
		this->_remotePort = NULL;
		this->_localHost = NULL;
		this->_localPort = NULL;
	}

	Connector::~Connector()
	{
		this->_thread->running = false;
		this->_thread->join();
		delete this->_thread;
	}

	void Connector::_integrate(State* stateValue, hmutex* mutexStateValue, Host* remoteHost, unsigned short* remotePort, Host* localHost, unsigned short* localPort)
	{
		this->_state = stateValue;
		this->_mutexState = mutexStateValue;
		this->_remoteHost = remoteHost;
		this->_remotePort = remotePort;
		this->_localHost = localHost;
		this->_localPort = localPort;
	}

	bool Connector::isConnecting()
	{
		this->_mutexState->lock();
		bool result = (*this->_state == CONNECTING);
		this->_mutexState->unlock();
		return result;
	}

	bool Connector::isConnected()
	{
		this->_mutexState->lock();
		bool result = (*this->_state != IDLE && *this->_state != CONNECTING);
		this->_mutexState->unlock();
		return result;
	}

	bool Connector::isDisconnecting()
	{
		this->_mutexState->lock();
		bool result = (*this->_state == DISCONNECTING);
		this->_mutexState->unlock();
		return result;
	}

	void Connector::_update(float timeSinceLastFrame)
	{
		this->_mutexState->lock();
		this->_thread->mutex.lock();
		State state = *this->_state;
		State result = this->_thread->result;
		Host remoteHost = this->_thread->host;
		unsigned short remotePort = this->_thread->port;
		Host localHost = this->_thread->localHost;
		unsigned short localPort = this->_thread->localPort;
		if (result == RUNNING || result == IDLE)
		{
			this->_thread->mutex.unlock();
			this->_mutexState->unlock();
			return;
		}
		this->_thread->state = IDLE;
		this->_thread->result = IDLE;
		switch (result)
		{
		case FINISHED:
			switch (state)
			{
			case CONNECTING:
				*this->_state = CONNECTED;
				*this->_remoteHost = remoteHost;
				*this->_remotePort = remotePort;
				*this->_localHost = localHost;
				*this->_localPort = localPort;
				break;
			case DISCONNECTING:
				*this->_state = IDLE;
				remoteHost = *this->_remoteHost;
				remotePort = *this->_remotePort;
				*this->_remoteHost = Host();
				*this->_remotePort = 0;
				*this->_localHost = Host();
				*this->_localPort = 0;
				break;
			}
			break;
		case FAILED:
			switch (state)
			{
			case CONNECTING:
				*this->_state = IDLE;
				break;
			case DISCONNECTING:
				*this->_state = CONNECTED;
				break;
			}
			break;
		}
		this->_thread->mutex.unlock();
		this->_mutexState->unlock();
		// delegate calls
		switch (result)
		{
		case FINISHED:
			switch (state)
			{
			case CONNECTING:	this->_connectorDelegate->onConnected(this, remoteHost, remotePort);		break;
			case DISCONNECTING:	this->_connectorDelegate->onDisconnected(this, remoteHost, remotePort);		break;
			}
			break;
		case FAILED:
			switch (state)
			{
			case CONNECTING:	this->_connectorDelegate->onConnectFailed(this, remoteHost, remotePort);	break;
			case DISCONNECTING:	this->_connectorDelegate->onDisconnectFailed(this, remoteHost, remotePort);	break;
			}
			break;
		}
	}

	bool Connector::connect(Host remoteHost, unsigned short remotePort)
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canConnect(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = CONNECTING;
		this->_mutexState->unlock();
		Host localHost;
		unsigned short localPort = 0;
		bool result = this->_socket->connect(remoteHost, remotePort, localHost, localPort);
		this->_mutexState->lock();
		if (result)
		{
			*this->_remoteHost = remoteHost;
			*this->_remotePort = remotePort;
			*this->_localHost = localHost;
			*this->_localPort = localPort;
			*this->_state = CONNECTED;
		}
		else
		{
			*this->_state = state;
		}
		this->_mutexState->unlock();
		return result;
	}
	
	bool Connector::disconnect()
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canDisconnect(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = DISCONNECTING;
		this->_mutexState->unlock();
		bool result = this->_socket->disconnect();
		this->_mutexState->lock();
		if (result)
		{
			*this->_remoteHost = Host();
			*this->_remotePort = 0;
			*this->_localHost = Host();
			*this->_localPort = 0;
			*this->_state = IDLE;
		}
		else
		{
			*this->_state = state;
		}
		this->_mutexState->unlock();
		return result;
	}

	bool Connector::connectAsync(Host remoteHost, unsigned short remotePort)
	{
		this->_mutexState->lock();
		if (!this->_canConnect(*this->_state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = CONNECTING;
		this->_thread->state = CONNECTING;
		this->_thread->result = RUNNING;
		this->_thread->host = remoteHost;
		this->_thread->port = remotePort;
		this->_mutexState->unlock();
		this->_thread->start();
		return true;
	}
	
	bool Connector::disconnectAsync()
	{
		this->_mutexState->lock();
		if (!this->_canDisconnect(*this->_state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = DISCONNECTING;
		this->_thread->state = DISCONNECTING;
		this->_thread->result = RUNNING;
		this->_mutexState->unlock();
		this->_thread->start();
		return true;
	}

	bool Connector::_canConnect(State state)
	{
		harray<State> allowed;
		allowed += IDLE;
		return _checkState(state, allowed, "connect");
	}

	bool Connector::_canDisconnect(State state)
	{
		harray<State> allowed;
		allowed += CONNECTED;
		return _checkState(state, allowed, "disconnect");
	}

}
