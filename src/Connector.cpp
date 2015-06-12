/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "Connector.h"
#include "ConnectorDelegate.h"
#include "ConnectorThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "sakitUtil.h"

namespace sakit
{
	Connector::Connector(PlatformSocket* socket, ConnectorDelegate* connectorDelegate)
	{
		this->_socket = socket;
		this->_connectorDelegate = connectorDelegate;
		this->_thread = NULL;
		this->_state = NULL;
		this->_mutexState = NULL;
		this->_remoteHost = NULL;
		this->_remotePort = NULL;
		this->_localHost = NULL;
		this->_localPort = NULL;
		this->_timeout = NULL;
		this->_retryFrequency = NULL;
	}

	Connector::~Connector()
	{
		if (this->_thread != NULL)
		{
			this->_thread->join();
			delete this->_thread;
		}
	}

	void Connector::_integrate(State* stateValue, hmutex* mutexStateValue, Host* remoteHost, unsigned short* remotePort, Host* localHost, unsigned short* localPort, float* timeout, float* retryFrequency)
	{
		this->_state = stateValue;
		this->_mutexState = mutexStateValue;
		this->_remoteHost = remoteHost;
		this->_remotePort = remotePort;
		this->_localHost = localHost;
		this->_localPort = localPort;
		this->_timeout = timeout;
		this->_retryFrequency = retryFrequency;
		this->_thread = new ConnectorThread(this->_socket, this->_timeout, this->_retryFrequency);
	}

	bool Connector::isConnecting()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state == CONNECTING);
	}

	bool Connector::isConnected()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state != IDLE && *this->_state != CONNECTING);
	}

	bool Connector::isDisconnecting()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state == DISCONNECTING);
	}

	void Connector::_update(float timeDelta)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		hmutex::ScopeLock lockThread(&this->_thread->mutex);
		State state = *this->_state;
		State result = this->_thread->result;
		Host remoteHost = this->_thread->host;
		unsigned short remotePort = this->_thread->port;
		Host localHost = this->_thread->localHost;
		unsigned short localPort = this->_thread->localPort;
		if (result == RUNNING || result == IDLE)
		{
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
			default:
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
			default:
				break;
			}
			break;
		default:
			break;
		}
		lockThread.release();
		lock.release();
		// delegate calls
		switch (result)
		{
		case FINISHED:
			switch (state)
			{
			case CONNECTING:	this->_connectorDelegate->onConnected(this, remoteHost, remotePort);		break;
			case DISCONNECTING:	this->_connectorDelegate->onDisconnected(this, remoteHost, remotePort);		break;
			default:																						break;
			}
			break;
		case FAILED:
			switch (state)
			{
			case CONNECTING:	this->_connectorDelegate->onConnectFailed(this, remoteHost, remotePort);	break;
			case DISCONNECTING:	this->_connectorDelegate->onDisconnectFailed(this, remoteHost, remotePort);	break;
			default:																						break;
			}
			break;
		default:
			break;
		}
	}

	bool Connector::connect(Host remoteHost, unsigned short remotePort)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		State state = *this->_state;
		if (!this->_canConnect(state))
		{
			return false;
		}
		*this->_state = CONNECTING;
		lock.release();
		Host localHost;
		unsigned short localPort = 0;
		bool result = this->_socket->connect(remoteHost, remotePort, localHost, localPort, *this->_timeout, *this->_retryFrequency);
		lock.acquire(this->_mutexState);
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
		return result;
	}
	
	bool Connector::disconnect()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		State state = *this->_state;
		if (!this->_canDisconnect(state))
		{
			return false;
		}
		*this->_state = DISCONNECTING;
		lock.release();
		bool result = this->_socket->disconnect();
		lock.acquire(this->_mutexState);
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
		return result;
	}

	bool Connector::connectAsync(Host remoteHost, unsigned short remotePort)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		if (!this->_canConnect(*this->_state))
		{
			return false;
		}
		*this->_state = CONNECTING;
		this->_thread->state = CONNECTING;
		this->_thread->result = RUNNING;
		this->_thread->host = remoteHost;
		this->_thread->port = remotePort;
		this->_thread->start();
		return true;
	}
	
	bool Connector::disconnectAsync()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		if (!this->_canDisconnect(*this->_state))
		{
			return false;
		}
		*this->_state = DISCONNECTING;
		this->_thread->state = DISCONNECTING;
		this->_thread->result = RUNNING;
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
