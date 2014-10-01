/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"
#include "SocketDelegate.h"
#include "TcpServerThread.h"
#include "TcpSocket.h"

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;

	TcpServerThread::TcpServerThread(PlatformSocket* socket, TcpSocketDelegate* acceptedDelegate, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency)
	{
		this->name = "SAKit TCP server";
		this->acceptedDelegate = acceptedDelegate;
	}

	TcpServerThread::~TcpServerThread()
	{
	}

	void TcpServerThread::_updateProcess()
	{
		TcpSocket* tcpSocket = new TcpSocket(this->acceptedDelegate);
		connectionsMutex.lock();
		connections -= tcpSocket;
		connectionsMutex.unlock();
		while (this->running)
		{
			if (!this->socket->listen())
			{
				this->mutex.lock();
				this->result = FAILED;
				this->mutex.unlock();
				delete tcpSocket;
				return;
			}
			if (this->socket->accept(tcpSocket))
			{
				this->mutex.lock();
				this->sockets += tcpSocket;
				this->mutex.unlock();
				tcpSocket = new TcpSocket(this->acceptedDelegate);
				connectionsMutex.lock();
				connections -= tcpSocket;
				connectionsMutex.unlock();
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		delete tcpSocket;
		this->mutex.lock();
		this->result = FINISHED;
		this->mutex.unlock();
	}

}
