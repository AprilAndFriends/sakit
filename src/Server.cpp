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
		this->basicThread->mutex.lock();
		bool result = (this->basicThread->state == RUNNING);
		this->basicThread->mutex.unlock();
		return result;
	}

	bool Server::startAsync()
	{
		// TODOsock
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		if (!this->_checkStartStatus(state))
		{
			this->basicThread->mutex.unlock();
			return false;
		}
		this->basicThread->state = RUNNING;
		this->basicThread->mutex.unlock();
		this->basicThread->start();
		return true;
	}

	bool Server::stopAsync()
	{
		// TODOsock
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		this->basicThread->mutex.unlock();
		if (!this->_checkStopStatus(state))
		{
			return false;
		}
		this->basicThread->running = false;
		return true;
	}

	void Server::update(float timeSinceLastFrame)
	{
		// TODOsock
		Binder::_update(timeSinceLastFrame);
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		State result = this->basicThread->result;
		Host host = this->basicThread->host;
		unsigned short port = this->basicThread->port;
		this->basicThread->mutex.unlock();
		if (result == RUNNING || result == IDLE)
		{
			return;
		}
		this->basicThread->mutex.lock();
		this->basicThread->result = IDLE;
		this->basicThread->mutex.unlock();
		if (result == FINISHED)
		{
			switch (state)
			{
				/*
			case BINDING:
				this->basicThread->mutex.lock();
				this->basicThread->state = BOUND;
				this->basicThread->mutex.unlock();
				this->host = host;
				this->port = port;
				this->basicDelegate->onBound(this);
				break;
			case UNBINDING:
				this->basicThread->mutex.lock();
				this->basicThread->state = IDLE;
				this->basicThread->mutex.unlock();
				host = this->host;
				port = this->port;
				this->host = Host();
				this->port = 0;
				this->basicDelegate->onUnbound(this, host, port);
				break;
				*/
			case RUNNING:
				this->basicThread->mutex.lock();
				this->basicThread->state = BOUND;
				this->basicThread->mutex.unlock();
				this->basicDelegate->onStopped(this);
				break;
			}
		}
		else if (result == FAILED)
		{
			switch (state)
			{
				/*
			case BINDING:
				this->basicThread->mutex.lock();
				this->basicThread->state = IDLE;
				this->basicThread->mutex.unlock();
				this->basicDelegate->onBindFailed(this, host, port);
				break;
			case UNBINDING:
				this->basicThread->mutex.lock();
				this->basicThread->state = BOUND;
				this->basicThread->mutex.unlock();
				this->basicDelegate->onUnbindFailed(this);
				break;
				*/
			case RUNNING:
				this->basicThread->mutex.lock();
				this->basicThread->state = BOUND;
				this->basicThread->mutex.unlock();
				this->basicDelegate->onRunFailed(this);
				break;
			}
		}
	}

	bool Server::_checkStartStatus(State state)
	{
		switch (state)
		{
		case IDLE:		hlog::warn(sakit::logTag, "Cannot start, not bound!");			return false;
		case BINDING:	hlog::warn(sakit::logTag, "Cannot start, still binding!");		return false;
		case RUNNING:	hlog::warn(sakit::logTag, "Cannot start, already running!");	return false;
		case UNBINDING:	hlog::warn(sakit::logTag, "Cannot start, already unbinding!");	return false;
		}
		return true;
	}

	bool Server::_checkStopStatus(State state)
	{
		switch (state)
		{
		case IDLE:		hlog::warn(sakit::logTag, "Cannot stop, not bound!");			return false;
		case BINDING:	hlog::warn(sakit::logTag, "Cannot stop, still binding!");		return false;
		case BOUND:		hlog::warn(sakit::logTag, "Cannot stop, not running!");			return false;
		case UNBINDING:	hlog::warn(sakit::logTag, "Cannot stop, already unbinding!");	return false;
		}
		return true;
	}

}
