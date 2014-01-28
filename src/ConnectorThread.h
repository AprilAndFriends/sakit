/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a thread for connecting/disconnecting a TCP socket.

#ifndef SAKIT_CONNECTOR_THREAD_H
#define SAKIT_CONNECTOR_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Server.h"
#include "Socket.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class ServerDelegate;
	class SocketDelegate;
	class TcpSocket;

	class ConnectorThread : public WorkerThread
	{
	public:
		friend class TcpSocket;

		ConnectorThread(PlatformSocket* socket);
		~ConnectorThread();

	protected:
		SocketBase::State state;

		void _updateConnecting();
		void _updateDisconnecting();
		void _updateProcess();

	};

}
#endif
