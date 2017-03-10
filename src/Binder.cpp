/// @file
/// @version 1.2
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
		return (*this->_state == State::Binding);
	}

	bool Binder::isBound()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state != State::Idle && *this->_state != State::Binding);
	}

	bool Binder::isUnbinding()
	{
		hmutex::ScopeLock lock(this->_mutexState);
		return (*this->_state == State::Unbinding);
	}

	void Binder::_update(float timeDelta)
	{
		hmutex::ScopeLock lock(this->_mutexState);
		hmutex::ScopeLock lockThreadResult(&this->_thread->resultMutex);
		State state = *this->_state;
		State result = this->_thread->result;
		Host localHost = this->_thread->host;
		unsigned short localPort = (unsigned short)(int)this->_thread->port;
		if (result == State::Running || result == State::Idle)
		{
			return;
		}
		this->_thread->state = State::Idle;
		this->_thread->result = State::Idle;
		if (result == State::Finished)
		{
			if (state == State::Binding)
			{
				*this->_state = State::Bound;
				*this->_localHost = localHost;
				*this->_localPort = localPort;
			}
			else if (state == State::Unbinding)
			{
				*this->_state = State::Idle;
				localHost = *this->_localHost;
				localPort = *this->_localPort;
				*this->_localHost = Host();
				*this->_localPort = 0;
			}
		}
		else if (result == State::Failed)
		{
			if (state == State::Binding)
			{
				*this->_state = State::Idle;
			}
			else if (state == State::Unbinding)
			{
				*this->_state = State::Bound;
			}
		}
		lockThreadResult.release();
		lock.release();
		// delegate calls
		if (result == State::Finished)
		{
			if (state == State::Binding)
			{
				this->_binderDelegate->onBound(this, localHost, localPort);
			}
			else if (state == State::Unbinding)
			{
				this->_binderDelegate->onUnbound(this, localHost, localPort);
			}
		}
		else if (result == State::Failed)
		{
			if (state == State::Binding)
			{
				this->_binderDelegate->onBindFailed(this, localHost, localPort);
			}
			else if (state == State::Unbinding)
			{
				this->_binderDelegate->onUnbindFailed(this, localHost, localPort);
			}
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
		*this->_state = State::Binding;
		lock.release();
		bool result = this->_socket->bind(localHost, localPort);
		lock.acquire(this->_mutexState);
		if (result)
		{
			*this->_localHost = localHost;
			*this->_localPort = localPort;
			*this->_state = State::Bound;
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
		*this->_state = State::Unbinding;
		lock.release();
		bool result = this->_socket->disconnect();
		lock.acquire(this->_mutexState);
		if (result)
		{
			*this->_localHost = Host();
			*this->_localPort = 0;
			*this->_state = State::Idle;
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
		*this->_state = State::Binding;
		this->_thread->state = State::Binding;
		this->_thread->result = State::Running;
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
		*this->_state = State::Unbinding;
		this->_thread->state = State::Unbinding;
		this->_thread->result = State::Running;
		this->_thread->start();
		return true;
	}

	bool Binder::_canBind(State state)
	{
		return _checkState(state, State::allowedBindStates, "bind");
	}

	bool Binder::_canUnbind(State state)
	{
		return _checkState(state, State::allowedUnbindStates, "unbind");
	}

}
