/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "sakitUtil.h"
#include "Server.h"
#include "ServerDelegate.h"
#include "Socket.h"
#include "TcpSocket.h"
#include "WorkerThread.h"

namespace sakit
{
	Server::Server(ServerDelegate* serverDelegate) : Base(), Binder(this->socket, dynamic_cast<BinderDelegate*>(serverDelegate))
	{
		this->serverDelegate = serverDelegate;
		this->serverThread = NULL;
		this->socket->setServerMode(true);
		Binder::_integrate(&this->state, &this->mutexState, &this->localHost, &this->localPort);
	}

	Server::~Server()
	{
		if (this->serverThread != NULL)
		{
			this->serverThread->join();
			delete this->serverThread;
		}
	}

	bool Server::isRunning()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		return (this->state == State::Running);
	}

	bool Server::startAsync()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canStart(this->state))
		{
			return false;
		}
		this->state = State::Running;
		this->serverThread->result = State::Running;
		this->serverThread->start();
		return true;
	}

	bool Server::stopAsync()
	{
		hmutex::ScopeLock lock(&this->mutexState);
		if (!this->_canStop(this->state))
		{
			return false;
		}
		this->serverThread->executing = false;
		return true;
	}

	void Server::update(float timeDelta)
	{
		Binder::_update(timeDelta);
		hmutex::ScopeLock lock(&this->mutexState);
		hmutex::ScopeLock lockThreadResult(&this->serverThread->resultMutex);
		State result = this->serverThread->result;
		if (result == State::Running || result == State::Idle)
		{
			return;
		}
		this->serverThread->result = State::Idle;
		this->state = State::Bound;
		lockThreadResult.release();
		lock.release();
		// delegate calls
		if (result == State::Finished)
		{
			this->serverDelegate->onStopped(this);
		}
		else if (result == State::Failed)
		{
			this->serverDelegate->onStartFailed(this);
		}
	}

	bool Server::_canStart(State state)
	{
		return _checkState(state, State::allowedServerStartStates, "start");
	}

	bool Server::_canStop(State state)
	{
		return _checkState(state, State::allowedServerStopStates, "stop");
	}

}
