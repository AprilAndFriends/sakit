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
#include "ServerThread.h"
#include "Socket.h"
#include "TcpSocket.h"

namespace sakit
{
	Server::Server(ServerDelegate* serverDelegate) : Base()
	{
		this->basicDelegate = serverDelegate;
		this->basicThread = NULL;
	}

	Server::~Server()
	{
		this->basicThread->running = false;
		this->basicThread->join();
		delete this->basicThread;
	}

	bool Server::isBinding()
	{
		this->basicThread->mutex.lock();
		bool result = (this->basicThread->state == BINDING);
		this->basicThread->mutex.unlock();
		return result;
	}

	bool Server::isBound()
	{
		this->basicThread->mutex.lock();
		bool result = (this->basicThread->state == BOUND || this->basicThread->state == UNBINDING || this->basicThread->state == RUNNING);
		this->basicThread->mutex.unlock();
		return result;
	}

	bool Server::isRunning()
	{
		this->basicThread->mutex.lock();
		bool result = (this->basicThread->state == RUNNING);
		this->basicThread->mutex.unlock();
		return result;
	}

	bool Server::isUnbinding()
	{
		this->basicThread->mutex.lock();
		bool result = (this->basicThread->state == UNBINDING);
		this->basicThread->mutex.unlock();
		return result;
	}

	bool Server::bind(Ip host, unsigned short port)
	{
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		this->basicThread->mutex.unlock();
		bool result = (this->_checkBindStatus(state) && this->socket->bind(host, port));
		if (result)
		{
			this->basicThread->mutex.lock();
			this->basicThread->state = BOUND;
			this->basicThread->mutex.unlock();
			this->host = host;
			this->port = port;
		}
		return result;
	}

	bool Server::unbind()
	{
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		this->basicThread->mutex.unlock();
		bool result = (this->_checkUnbindStatus(state) && this->socket->disconnect());
		if (result)
		{
			this->basicThread->mutex.lock();
			this->basicThread->state = IDLE;
			this->basicThread->mutex.unlock();
			this->host = Ip();
			this->port = 0;
		}
		return result;
	}

	bool Server::bindAsync(Ip host, unsigned short port)
	{
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		if (!this->_checkBindStatus(state))
		{
			this->basicThread->mutex.unlock();
			return false;
		}
		this->basicThread->host = host;
		this->basicThread->port = port;
		this->basicThread->state = BINDING;
		this->basicThread->mutex.unlock();
		this->basicThread->start();
		return true;
	}

	bool Server::unbindAsync()
	{
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		if (!this->_checkUnbindStatus(state))
		{
			this->basicThread->mutex.unlock();
			return false;
		}
		this->basicThread->state = UNBINDING;
		this->basicThread->mutex.unlock();
		this->basicThread->start();
		return true;
	}

	bool Server::startAsync()
	{
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
		this->basicThread->mutex.lock();
		State state = this->basicThread->state;
		WorkerThread::Result result = this->basicThread->result;
		Ip host = this->basicThread->host;
		unsigned short port = this->basicThread->port;
		this->basicThread->mutex.unlock();
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			return;
		}
		this->basicThread->mutex.lock();
		this->basicThread->result = WorkerThread::IDLE;
		this->basicThread->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			switch (state)
			{
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
				this->host = Ip();
				this->port = 0;
				this->basicDelegate->onUnbound(this, host, port);
				break;
			case RUNNING:
				this->basicThread->mutex.lock();
				this->basicThread->state = BOUND;
				this->basicThread->mutex.unlock();
				this->basicDelegate->onStopped(this);
				break;
			}
		}
		else if (state == WorkerThread::FAILED)
		{
			switch (state)
			{
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
			case RUNNING:
				this->basicThread->mutex.lock();
				this->basicThread->state = BOUND;
				this->basicThread->mutex.unlock();
				this->basicDelegate->onRunFailed(this);
				break;
			}
		}
	}

	bool Server::_checkBindStatus(State state)
	{
		switch (state)
		{
		case BINDING:	hlog::warn(sakit::logTag, "Cannot bind, already binding!");		return false;
		case BOUND:		hlog::warn(sakit::logTag, "Cannot bind, already bound!");		return false;
		case RUNNING:	hlog::warn(sakit::logTag, "Cannot bind, already running!");		return false;
		case UNBINDING:	hlog::warn(sakit::logTag, "Cannot bind, already unbinding!");	return false;
		}
		return true;
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

	bool Server::_checkUnbindStatus(State state)
	{
		switch (state)
		{
		case IDLE:		hlog::warn(sakit::logTag, "Cannot unbind, not bound!");			return false;
		case BINDING:	hlog::warn(sakit::logTag, "Cannot unbind, still binding!");		return false;
		case RUNNING:	hlog::warn(sakit::logTag, "Cannot unbind, already running!");	return false;
		case UNBINDING:	hlog::warn(sakit::logTag, "Cannot unbind, already unbinding!");	return false;
		}
		return true;
	}

}
