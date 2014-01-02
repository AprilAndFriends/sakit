/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "AccepterThread.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Server.h"
#include "ServerDelegate.h"

namespace sakit
{
	Server::Server(ServerDelegate* serverDelegate, SocketDelegate* socketDelegate, int maxConnections) : Base()
	{
		this->serverDelegate = serverDelegate;
		this->socketDelegate = socketDelegate;
		this->maxConnections = hmax(maxConnections, 1);
		this->accepter = new AccepterThread(this->socket, this->socketDelegate);
	}

	Server::~Server()
	{
		this->accepter->running = false;
		this->accepter->join();
		delete this->accepter;
	}

	bool Server::isBound()
	{
		return this->socket->isConnected();
	}

	bool Server::bind(Ip host, unsigned short port)
	{
		if (this->unbind()) // unbind first
		{
			hlog::warn(sakit::logTag, "Server already bound, it was unbound.");
		}
		bool result = this->socket->bind(host, port);
		if (result)
		{
			this->host = host;
			this->port = port;
		}
		return result;
	}

	bool Server::unbind()
	{
		this->host = Ip("");
		this->port = 0;
		return this->socket->disconnect();
	}

	void Server::start()
	{
		if (!this->isBound())
		{
			hlog::error(sakit::logTag, "Not bound!");
			return;
		}
		this->accepter->mutex.lock();
		if (this->accepter->state != WorkerThread::IDLE)
		{
			hlog::warn(sakit::logTag, "Cannot start, already running!");
			this->accepter->mutex.unlock();
			return;
		}
		this->accepter->state = WorkerThread::RUNNING;
		/*
		long position = stream->position();
		stream->rewind();
		this->accepter->stream->clear();
		this->accepter->stream->write_raw(*stream, hmin((long)maxBytes, stream->size()));
		this->accepter->stream->rewind();
		*/
		this->accepter->mutex.unlock();
		this->accepter->start();
		//stream->seek(position, hsbase::START);

	}

	void Server::stop()
	{
		this->accepter->running = false;
	}

	void Server::update(float timeSinceLastFrame)
	{
		this->accepter->mutex.lock();
		WorkerThread::State state = this->accepter->state;
		if (this->accepter->sockets.size() > 0)
		{
			harray<Socket*> sockets = this->accepter->sockets;
			this->accepter->sockets.clear();
			this->accepter->mutex.unlock();
			this->sockets += sockets;
			foreach (Socket*, it, sockets)
			{
				this->serverDelegate->onAccepted(this, (*it));
			}
		}
		else
		{
			this->accepter->mutex.unlock();
		}
		if (state == WorkerThread::RUNNING || state == WorkerThread::IDLE)
		{
			return;
		}
		/*
		this->accepter->mutex.lock();
		this->accepter->state = WorkerThread::IDLE;
		this->accepter->mutex.unlock();
		if (state == WorkerThread::FINISHED)
		{
			this->serverDelegate->onSendFinished(this);
		}
		else if (state == WorkerThread::FAILED)
		{
			this->serverDelegate->onSendFailed(this);
		}
		*/
	}
	
}
