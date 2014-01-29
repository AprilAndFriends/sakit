/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
		this->basicDelegate = serverDelegate;
		this->basicThread = NULL;
		Binder::_integrate(&this->state, &this->mutexState, &this->host, &this->port);
	}

	Server::~Server()
	{
		this->basicThread->running = false;
		this->basicThread->join();
		delete this->basicThread;
	}

	bool Server::isRunning()
	{
		this->mutexState.lock();
		bool result = (this->state == RUNNING);
		this->mutexState.unlock();
		return result;
	}

	bool Server::startAsync()
	{
		this->mutexState.lock();
		State state = this->state;
		if (!this->_canStart(state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->state = RUNNING;
		this->basicThread->result = RUNNING;
		this->mutexState.unlock();
		this->basicThread->start();
		return true;
	}

	bool Server::stopAsync()
	{
		this->mutexState.lock();
		State state = this->state;
		if (!this->_canStop(state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->basicThread->running = false;
		this->mutexState.unlock();
		return true;
	}

	void Server::update(float timeSinceLastFrame)
	{
		// TODOsock
		Binder::_update(timeSinceLastFrame);
		this->mutexState.lock();
		this->basicThread->mutex.lock();
		State state = this->state;
		State result = this->basicThread->result;
		if (result == RUNNING || result == IDLE)
		{
			this->basicThread->mutex.unlock();
			this->mutexState.unlock();
			return;
		}
		this->basicThread->result = IDLE;
		this->state = BOUND;
		this->basicThread->mutex.unlock();
		this->mutexState.unlock();
		// delegate calls
		switch (result)
		{
		case FINISHED:
			this->basicDelegate->onStopped(this);
			break;
		case FAILED:
			this->basicDelegate->onStartFailed(this);
			break;
		}
	}

	bool Server::_canStart(State state)
	{
		harray<State> allowed;
		allowed += BOUND;
		return _checkState(state, allowed, "start");
	}

	bool Server::_canStop(State state)
	{
		harray<State> allowed;
		allowed += RUNNING;
		return _checkState(state, allowed, "stop");
	}

}
