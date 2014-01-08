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
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "SenderThread.h"
#include "UdpServer.h"
#include "UdpServerDelegate.h"
#include "UdpServerThread.h"
#include "UdpSocket.h"

namespace sakit
{
	UdpServer::UdpServer(UdpServerDelegate* serverDelegate, SocketDelegate* socketDelegate) : Server(serverDelegate)
	{
		this->basicThread = this->thread = new UdpServerThread(this->socket);
		this->sender = new SenderThread(this->socket);
		this->basicDelegate = this->serverDelegate = serverDelegate;
		this->socket->setConnectionLess(true);

	}

	UdpServer::~UdpServer()
	{
		delete this->sender;
	}
	
	void UdpServer::update(float timeSinceLastFrame)
	{
		this->thread->mutex.lock();
		if (this->thread->streams.size() > 0)
		{
			harray<hstream*> streams = this->thread->streams;
			harray<Ip> hosts = this->thread->hosts;
			harray<unsigned short> ports = this->thread->ports;
			this->thread->streams.clear();
			this->thread->hosts.clear();
			this->thread->ports.clear();
			this->thread->mutex.unlock();
			for_iter (i, 0, streams.size())
			{
				this->serverDelegate->onReceived(this, streams[i], hosts[i], ports[i]);
				delete streams[i];
			}
		}
		else
		{
			this->thread->mutex.unlock();
		}
		// TODOsock - implement a different sender for UDP servers
		/*
		this->sender->mutex.lock();
		Ip host = this->sender->host;
		unsigned short port = this->sender->port;
		WorkerThread::Result result = this->sender->result;
		if (this->sender->lastSent > 0)
		{
			int sent = this->sender->lastSent;
			this->sender->lastSent = 0;
			this->sender->mutex.unlock();
			this->serverDelegate->onSent(this, host, port, sent);
		}
		else
		{
			this->sender->mutex.unlock();
		}
		if (result == WorkerThread::RUNNING || result == WorkerThread::IDLE)
		{
			return;
		}
		this->sender->mutex.lock();
		this->sender->result = WorkerThread::IDLE;
		this->sender->state = Socket::IDLE;
		this->sender->mutex.unlock();
		if (result == WorkerThread::FINISHED)
		{
			this->serverDelegate->onSendFinished(this, host, port);
		}
		else if (result == WorkerThread::FAILED)
		{
			this->serverDelegate->onSendFailed(this, host, port);
		}
		*/
		Server::update(timeSinceLastFrame);
	}

	int UdpServer::send(Ip host, unsigned short port, hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		this->sender->mutex.lock();
		Socket::State senderState = this->sender->state;
		this->sender->mutex.unlock();
		if (!this->_checkSendStatus(senderState))
		{
			return false;
		}
		return this->_send(stream, count);
	}

	int UdpServer::send(Ip host, unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->send(host, port, &stream);
	}

	bool UdpServer::receive(hstream* stream, Ip* host, unsigned short* port)
	{
		this->thread->mutex.lock();
		State state = this->thread->state;
		this->thread->mutex.unlock();
		return (this->_checkStartStatus(state) && this->socket->receiveFrom(stream, host, port));
	}

	bool UdpServer::sendAsync(Ip host, unsigned short port, hstream* stream, int count)
	{
		if (!this->_canSend(stream, count))
		{
			return false;
		}
		this->sender->mutex.lock();
		Socket::State senderState = this->sender->state;
		if (!this->_checkSendStatus(senderState))
		{
			this->sender->mutex.unlock();
			return false;
		}
		this->sender->stream->clear();
		this->sender->stream->write_raw(*stream, hmin((long)count, stream->size() - stream->position()));
		this->sender->stream->rewind();
		this->sender->state = Socket::RUNNING;
		this->sender->mutex.unlock();
		this->sender->start();
		return true;
	}

	bool UdpServer::sendAsync(Ip host, unsigned short port, chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->sendAsync(host, port, &stream);
	}

	bool UdpServer::_checkSendStatus(Socket::State senderState)
	{
		if (senderState == Socket::RUNNING)
		{
			hlog::warn(sakit::logTag, "Cannot send, already sending!");
			return false;
		}
		return true;
	}

}
