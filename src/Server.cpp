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

namespace sakit
{
	Server::Server(ServerDelegate* serverDelegate, SocketDelegate* acceptedDelegate) : Base()
	{
		this->serverDelegate = serverDelegate;
		this->acceptedDelegate = acceptedDelegate;
		this->thread = new ServerThread(this->socket, this->acceptedDelegate);
	}

	Server::~Server()
	{
		this->thread->running = false;
		this->thread->join();
		delete this->thread;
		foreach (Socket*, it, this->sockets)
		{
			delete (*it);
		}
	}

	bool Server::isBinding()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == BINDING);
		this->thread->mutex.unlock();
		return result;
	}

	bool Server::isBound()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == BOUND || this->thread->state == UNBINDING || this->thread->state == RUNNING);
		this->thread->mutex.unlock();
		return result;
	}

	bool Server::isRunning()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == RUNNING);
		this->thread->mutex.unlock();
		return result;
	}

	bool Server::isUnbinding()
	{
		this->thread->mutex.lock();
		bool result = (this->thread->state == UNBINDING);
		this->thread->mutex.unlock();
		return result;
	}

	bool Server::bind(Ip host, unsigned short port)
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		bool result = (this->_checkBindStatus(state) && this->socket->bind(host, port));
		if (result)
		{
			this->thread->mutex.lock();
			this->thread->state = BOUND;
			this->thread->mutex.unlock();
			this->host = host;
			this->port = port;
		}
		return result;
	}

	bool Server::unbind()
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		bool result = (this->_checkUnbindStatus(state) && this->socket->disconnect());
		if (result)
		{
			this->thread->mutex.lock();
			this->thread->state = IDLE;
			this->thread->mutex.unlock();
			this->host = Ip("");
			this->port = 0;
		}
		return result;
	}

	Socket* Server::accept(float timeout)
	{
		Socket* socket = NULL;
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		if (this->_checkStartStatus(state))
		{
			float retryTimeout = sakit::getRetryTimeout() * 1000.0f;
			timeout *= 1000.0f;
			while (timeout > 0.0f)
			{
				if (!this->socket->listen())
				{
					break;
				}
				socket = new Socket(this->acceptedDelegate);
				if (this->socket->accept(socket))
				{
					this->sockets += socket;
					break;
				}
				delete socket;
				socket = NULL;
				timeout -= retryTimeout;
				hthread::sleep(retryTimeout);
			}
		}
		return socket;
	}

	bool Server::destroy(Socket* socket)
	{
		// TODOsock - refactor this mechanism to have a safer environment for server-created sockets
		// (problems during sakit::update() and destruction of sockets in delegate callbacks)
		if (socket == NULL)
		{
			hlog::error(sakit::logTag, "Socket is NULL!");
			return false;
		}
		if (!this->sockets.contains(socket))
		{
			hlog::error(sakit::logTag, "Socket does not belong to the given server!");
			return false;
		}
		this->sockets -= socket;
		delete socket;
		return true;
	}

	bool Server::bindAsync(Ip host, unsigned short port)
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		if (!this->_checkBindStatus(state))
		{
			this->thread->mutex.unlock();
			return false;
		}
		this->thread->host = host;
		this->thread->port = port;
		this->thread->state = BINDING;
		this->thread->mutex.unlock();
		this->thread->start();
		return true;
	}

	bool Server::unbindAsync()
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		if (!this->_checkUnbindStatus(state))
		{
			this->thread->mutex.unlock();
			return false;
		}
		this->thread->state = UNBINDING;
		this->thread->mutex.unlock();
		this->thread->start();
		return true;
	}

	bool Server::startAsync()
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		if (!this->_checkStartStatus(state))
		{
			this->thread->mutex.unlock();
			return false;
		}
		this->thread->state = RUNNING;
		this->thread->mutex.unlock();
		this->thread->start();
		return true;
	}

	bool Server::stopAsync()
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		if (!this->_checkStopStatus(state))
		{
			return false;
		}
		this->thread->running = false;
		return true;
	}

	void Server::update(float timeSinceLastFrame)
	{
		harray<Socket*> sockets = this->sockets;
		this->sockets.clear();
		foreach (Socket*, it, sockets)
		{
			if ((*it)->isConnected())
			{
				this->sockets += (*it);
			}
			else
			{
				delete (*it);
			}
		}
		this->thread->mutex.lock();
		State state = this->thread->state;
		WorkerThread::Result result = this->thread->result;
		Ip host = this->thread->host;
		unsigned short port = this->thread->port;
		if (this->thread->sockets.size() > 0)
		{
			harray<Socket*> sockets = this->thread->sockets;
			this->thread->sockets.clear();
			this->thread->mutex.unlock();
			this->sockets += sockets;
			foreach (Socket*, it, sockets)
			{
				this->serverDelegate->onAccepted(this, (*it));
			}
		}
		else
		{
			this->thread->mutex.unlock();
		}
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			return;
		}
		this->thread->mutex.lock();
		this->thread->result = WorkerThread::IDLE;
		this->thread->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			switch (state)
			{
			case BINDING:
				this->thread->mutex.lock();
				this->thread->state = BOUND;
				this->thread->mutex.unlock();
				this->host = host;
				this->port = port;
				this->serverDelegate->onBound(this);
				break;
			case UNBINDING:
				this->thread->mutex.lock();
				this->thread->state = IDLE;
				this->thread->mutex.unlock();
				host = this->host;
				port = this->port;
				this->host = Ip("");
				this->port = 0;
				this->serverDelegate->onUnbound(this, host, port);
				break;
			case RUNNING:
				this->thread->mutex.lock();
				this->thread->state = BOUND;
				this->thread->mutex.unlock();
				this->serverDelegate->onStopped(this);
				break;
			}
		}
		else if (state == WorkerThread::FAILED)
		{
			switch (state)
			{
			case BINDING:
				this->thread->mutex.lock();
				this->thread->state = IDLE;
				this->thread->mutex.unlock();
				this->serverDelegate->onBindFailed(this, host, port);
				break;
			case UNBINDING:
				this->thread->mutex.lock();
				this->thread->state = BOUND;
				this->thread->mutex.unlock();
				this->serverDelegate->onUnbindFailed(this);
				break;
			case RUNNING:
				this->thread->mutex.lock();
				this->thread->state = BOUND;
				this->thread->mutex.unlock();
				this->serverDelegate->onRunFailed(this);
				break;
			}
		}
	}
	
	bool Server::_checkBindStatus(State state)
	{
		switch (state)
		{
		case BINDING:
			hlog::warn(sakit::logTag, "Cannot bind, already binding!");
			return false;
		case BOUND:
			hlog::warn(sakit::logTag, "Cannot bind, already bound!");
			return false;
		case RUNNING:
			hlog::warn(sakit::logTag, "Cannot bind, already running!");
			return false;
		case UNBINDING:
			hlog::warn(sakit::logTag, "Cannot bind, already unbinding!");
			return false;
		}
		return true;
	}

	bool Server::_checkStartStatus(State state)
	{
		switch (state)
		{
		case IDLE:
			hlog::warn(sakit::logTag, "Cannot start, not bound!");
			return false;
		case BINDING:
			hlog::warn(sakit::logTag, "Cannot start, still binding!");
			return false;
		case RUNNING:
			hlog::warn(sakit::logTag, "Cannot start, already running!");
			return false;
		case UNBINDING:
			hlog::warn(sakit::logTag, "Cannot start, already unbinding!");
			return false;
		}
		return true;
	}

	bool Server::_checkStopStatus(State state)
	{
		switch (state)
		{
		case IDLE:
			hlog::warn(sakit::logTag, "Cannot stop, not bound!");
			return false;
		case BINDING:
			hlog::warn(sakit::logTag, "Cannot stop, still binding!");
			return false;
		case BOUND:
			hlog::warn(sakit::logTag, "Cannot stop, not running!");
			return false;
		case UNBINDING:
			hlog::warn(sakit::logTag, "Cannot stop, already unbinding!");
			return false;
		}
		return true;
	}

	bool Server::_checkUnbindStatus(State state)
	{
		switch (state)
		{
		case IDLE:
			hlog::warn(sakit::logTag, "Cannot unbind, not bound!");
			return false;
		case BINDING:
			hlog::warn(sakit::logTag, "Cannot unbind, still binding!");
			return false;
		case RUNNING:
			hlog::warn(sakit::logTag, "Cannot unbind, already running!");
			return false;
		case UNBINDING:
			hlog::warn(sakit::logTag, "Cannot unbind, already unbinding!");
			return false;
		}
		return true;
	}

}
