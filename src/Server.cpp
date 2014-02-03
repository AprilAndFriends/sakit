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
		this->serverDelegate = serverDelegate;
		this->serverThread = NULL;
		this->socket->setServerMode(true);
		Binder::_integrate(&this->state, &this->mutexState, &this->localHost, &this->localPort);
	}

	Server::~Server()
	{
		this->serverThread->join();
		delete this->serverThread;
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
		if (!this->_canStart(this->state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->state = RUNNING;
		this->serverThread->result = RUNNING;
		this->mutexState.unlock();
		this->serverThread->start();
		return true;
	}

	bool Server::stopAsync()
	{
		this->mutexState.lock();
		if (!this->_canStop(this->state))
		{
			this->mutexState.unlock();
			return false;
		}
		this->serverThread->running = false;
		this->mutexState.unlock();
		return true;
	}

	void Server::update(float timeSinceLastFrame)
	{
		Binder::_update(timeSinceLastFrame);
		this->mutexState.lock();
		this->serverThread->mutex.lock();
		State result = this->serverThread->result;
		if (result == RUNNING || result == IDLE)
		{
			this->serverThread->mutex.unlock();
			this->mutexState.unlock();
			return;
		}
		this->serverThread->result = IDLE;
		this->state = BOUND;
		this->serverThread->mutex.unlock();
		this->mutexState.unlock();
		// delegate calls
		switch (result)
		{
		case FINISHED:	this->serverDelegate->onStopped(this);		break;
		case FAILED:	this->serverDelegate->onStartFailed(this);	break;
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
