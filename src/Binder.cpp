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
		this->_thread = new BinderThread(this->_socket);
		this->_state = NULL;
		this->_mutexState = NULL;
		this->_host = NULL;
		this->_port = NULL;
	}

	Binder::~Binder()
	{
		this->_thread->running = false;
		this->_thread->join();
		delete this->_thread;
	}

	void Binder::_integrate(State* stateValue, hmutex* mutexStateValue, Host* host, unsigned short* port)
	{
		this->_state = stateValue;
		this->_mutexState = mutexStateValue;
		this->_host = host;
		this->_port = port;
	}

	bool Binder::isBinding()
	{
		this->_mutexState->lock();
		bool result = (*this->_state == BINDING);
		this->_mutexState->unlock();
		return result;
	}

	bool Binder::isBound()
	{
		this->_mutexState->lock();
		bool result = (*this->_state != IDLE && *this->_state != BINDING);
		this->_mutexState->unlock();
		return result;
	}

	bool Binder::isUnbinding()
	{
		this->_mutexState->lock();
		bool result = (*this->_state == UNBINDING);
		this->_mutexState->unlock();
		return result;
	}

	void Binder::_update(float timeSinceLastFrame)
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
			case BINDING:
				*this->_state = BOUND;
				*this->_host = host;
				*this->_port = port;
				break;
			case UNBINDING:
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
			case BINDING:
				*this->_state = IDLE;
				break;
			case UNBINDING:
				*this->_state = BOUND;
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
			case BINDING:	this->_binderDelegate->onBound(this, host, port);		break;
			case UNBINDING:	this->_binderDelegate->onUnbound(this, host, port);		break;
			}
			break;
		case FAILED:
			switch (state)
			{
			case BINDING:	this->_binderDelegate->onBindFailed(this, host, port);	break;
			case UNBINDING:	this->_binderDelegate->onUnbindFailed(this, host, port);	break;
			}
			break;
		}
	}

	bool Binder::bind(Host host, unsigned short port)
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canBind(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = BINDING;
		this->_mutexState->unlock();
		bool result = this->_socket->bind(host, port);
		this->_mutexState->lock();
		if (result)
		{
			*this->_host = host;
			*this->_port = port;
			*this->_state = BOUND;
		}
		else
		{
			*this->_state = state;
		}
		this->_mutexState->unlock();
		return result;
	}
	
	bool Binder::bind(unsigned short port)
	{
		return this->bind(Host::Any, port);
	}
	
	bool Binder::unbind()
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canUnbind(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = UNBINDING;
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

	bool Binder::bindAsync(Host host, unsigned short port)
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canBind(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = BINDING;
		this->_thread->state = BINDING;
		this->_thread->result = RUNNING;
		this->_thread->host = host;
		this->_thread->port = port;
		this->_mutexState->unlock();
		this->_thread->start();
		return true;
	}
	
	bool Binder::bindAsync(unsigned short port)
	{
		return this->bindAsync(Host::Any, port);
	}
	
	bool Binder::unbindAsync()
	{
		this->_mutexState->lock();
		State state = *this->_state;
		if (!this->_canUnbind(state))
		{
			this->_mutexState->unlock();
			return false;
		}
		*this->_state = UNBINDING;
		this->_thread->state = UNBINDING;
		this->_thread->result = RUNNING;
		this->_mutexState->unlock();
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
