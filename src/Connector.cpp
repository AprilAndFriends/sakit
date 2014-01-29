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
		this->_host = NULL;
		this->_port = NULL;
	}

	Connector::~Connector()
	{
		this->_thread->running = false;
		this->_thread->join();
		delete this->_thread;
	}

	void Connector::_integrate(State* stateValue, hmutex* mutexStateValue, Host* host, unsigned short* port)
	{
		this->_state = stateValue;
		this->_mutexState = mutexStateValue;
		this->_host = host;
		this->_port = port;
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
		Host host = this->_thread->host;
		unsigned short port = this->_thread->port;
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
				*this->_host = host;
				*this->_port = port;
				break;
			case DISCONNECTING:
				*this->_state = IDLE;
				host = *this->_host;
				port = *this->_port;
				*this->_host = Host();
				*this->_port = 0;
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
			case CONNECTING:	this->_connectorDelegate->onConnected(this, host, port);		break;
			case DISCONNECTING:	this->_connectorDelegate->onDisconnected(this, host, port);		break;
			}
			break;
		case FAILED:
			switch (state)
			{
			case CONNECTING:	this->_connectorDelegate->onConnectFailed(this, host, port);	break;
			case DISCONNECTING:	this->_connectorDelegate->onDisconnectFailed(this, host, port);	break;
			}
			break;
		}
	}

	bool Connector::connect(Host host, unsigned short port)
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
		bool result = this->_socket->connect(host, port);
		this->_mutexState->lock();
		if (result)
		{
			*this->_host = host;
			*this->_port = port;
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
			*this->_host = Host();
			*this->_port = 0;
			*this->_state = IDLE;
		}
		else
		{
			*this->_state = state;
		}
		this->_mutexState->unlock();
		return result;
	}

	bool Connector::connectAsync(Host host, unsigned short port)
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canConnect(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = CONNECTING;
		this->_thread->state = CONNECTING;
		this->_thread->result = RUNNING;
		this->_thread->host = host;
		this->_thread->port = port;
		this->_mutexState->unlock();
		this->_thread->start();
		return true;
	}
	
	bool Connector::disconnectAsync()
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canDisconnect(state))
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
