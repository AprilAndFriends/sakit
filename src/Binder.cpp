/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "Binder.h"
#include "BinderDelegate.h"
#include "BinderThread.h"
#include "PlatformSocket.h"
#include "sakitUtil.h"

namespace sakit
{
	Binder::Binder(PlatformSocket* socket, BinderDelegate* binderDelegate)
	{
		this->_socket = socket;
		this->_binderDelegate = binderDelegate;
		this->_thread = NULL;
		this->_state = NULL;
		this->_mutexState = NULL;
		this->_localHost = NULL;
		this->_localPort = NULL;
	}

	Binder::~Binder()
	{
		if (this->_thread != NULL)
		{
			this->_thread->join();
			delete this->_thread;
		}
	}

	void Binder::_integrate(State* stateValue, hmutex* mutexStateValue, Host* localHost, unsigned short* localPort)
	{
		this->_state = stateValue;
		this->_mutexState = mutexStateValue;
		this->_localHost = localHost;
		this->_localPort = localPort;
		this->_thread = new BinderThread(this->_socket);
	}

	bool Binder::isBinding()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state == BINDING);
	}

	bool Binder::isBound()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state != IDLE && *this->_state != BINDING);
	}

	bool Binder::isUnbinding()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state == UNBINDING);
	}

	void Binder::_update(float timeDelta)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		hmutex::ScopeLock lockThreadResult(&this->_thread->resultMutex);
		State state = *this->_state;
		State result = this->_thread->result;
		Host localHost = this->_thread->host;
		unsigned short localPort = (unsigned short)(int)this->_thread->port;
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
			case BINDING:
				*this->_state = BOUND;
				*this->_localHost = localHost;
				*this->_localPort = localPort;
				break;
			case UNBINDING:
				*this->_state = IDLE;
				localHost = *this->_localHost;
				localPort = *this->_localPort;
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
			case BINDING:
				*this->_state = IDLE;
				break;
			case UNBINDING:
				*this->_state = BOUND;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		lockThreadResult.release();
		lock.release();
		// delegate calls
		switch (result)
		{
		case FINISHED:
			switch (state)
			{
			case BINDING:	this->_binderDelegate->onBound(this, localHost, localPort);		break;
			case UNBINDING:	this->_binderDelegate->onUnbound(this, localHost, localPort);	break;
			default:																		break;
			}
			break;
		case FAILED:
			switch (state)
			{
			case BINDING:	this->_binderDelegate->onBindFailed(this, localHost, localPort);	break;
			case UNBINDING:	this->_binderDelegate->onUnbindFailed(this, localHost, localPort);	break;
			default:																			break;
			}
			break;
		default:
			break;
		}
	}

	bool Binder::bind(Host localHost, unsigned short localPort)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		State state = *this->_state;
		if (!this->_canBind(state))
		{
			return false;
		}
		*this->_state = BINDING;
		lock.release();
		bool result = this->_socket->bind(localHost, localPort);
		lock.acquire(this->_mutexState);
		if (result)
		{
			*this->_localHost = localHost;
			*this->_localPort = localPort;
			*this->_state = BOUND;
		}
		else
		{
			*this->_state = state;
		}
		return result;
	}
	
	bool Binder::bind(unsigned short localPort)
	{
		return this->bind(Host::Any, localPort);
	}
	
	bool Binder::unbind()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		State state = *this->_state;
		if (!this->_canUnbind(state))
		{
			return false;
		}
		*this->_state = UNBINDING;
		lock.release();
		bool result = this->_socket->disconnect();
		lock.acquire(this->_mutexState);
		if (result)
		{
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

	bool Binder::bindAsync(Host localHost, unsigned short localPort)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		if (!this->_canBind(*this->_state))
		{
			return false;
		}
		*this->_state = BINDING;
		this->_thread->state = BINDING;
		this->_thread->result = RUNNING;
		this->_thread->host = localHost;
		this->_thread->port = localPort;
		this->_thread->start();
		return true;
	}
	
	bool Binder::bindAsync(unsigned short localPort)
	{
		return this->bindAsync(Host::Any, localPort);
	}
	
	bool Binder::unbindAsync()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		if (!this->_canUnbind(*this->_state))
		{
			return false;
		}
		*this->_state = UNBINDING;
		this->_thread->state = UNBINDING;
		this->_thread->result = RUNNING;
		this->_thread->start();
		return true;
	}

	bool Binder::_canBind(State state)
	{
		harray<State> allowed;
		allowed += IDLE;
		return _checkState(state, allowed, "bind");
	}

	bool Binder::_canUnbind(State state)
	{
		harray<State> allowed;
		allowed += BOUND;
		return _checkState(state, allowed, "unbind");
	}

}
