/// @file
/// @version 1.1
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
	extern hmutex updateMutex;

	TcpServerThread::TcpServerThread(PlatformSocket* socket, TcpSocketDelegate* acceptedDelegate, float* timeout, float* retryFrequency) : TimedThread(socket, timeout, retryFrequency)
	{
		this->name = "SAKit TCP server";
		this->acceptedDelegate = acceptedDelegate;
	}

	TcpServerThread::~TcpServerThread()
	{
		hmutex::ScopeLock lock(&this->socketsMutex);
		harray<TcpSocket*> sockets = this->sockets;
		this->sockets.clear();
		lock.release();
		foreach (TcpSocket*, it, sockets)
		{
			delete (*it);
		}
	}

	void TcpServerThread::_updateProcess()
	{
		TcpSocket* tcpSocket = new TcpSocket(this->acceptedDelegate);
		hmutex::ScopeLock lockUpdate(&updateMutex);
		hmutex::ScopeLock lock(&connectionsMutex);
		connections -= tcpSocket;
		lock.release();
		lockUpdate.release();
		while (this->isRunning() && this->executing)
		{
			if (!this->socket->listen())
			{
				lock.acquire(&this->resultMutex);
				this->result = State::Failed;
				lock.release();
				delete tcpSocket;
				return;
			}
			if (this->socket->accept(tcpSocket))
			{
				lock.acquire(&this->socketsMutex);
				this->sockets += tcpSocket;
				lock.release();
				tcpSocket = new TcpSocket(this->acceptedDelegate);
				lockUpdate.acquire(&updateMutex);
				lock.acquire(&connectionsMutex);
				connections -= tcpSocket;
				lock.release();
				lockUpdate.release();
			}
			hthread::sleep(*this->retryFrequency * 1000.0f);
		}
		delete tcpSocket;
		lock.acquire(&this->resultMutex);
		this->result = State::Finished;
	}

}
