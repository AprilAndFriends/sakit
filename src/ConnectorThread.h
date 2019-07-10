/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a thread for connecting/disconnecting a TCP socket.

#ifndef SAKIT_CONNECTOR_THREAD_H
#define SAKIT_CONNECTOR_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Host.h"
#include "State.h"
#include "TimedThread.h"

namespace sakit
{
	class PlatformSocket;
	class Connector;

	class ConnectorThread : public TimedThread
	{
	public:
		friend class Connector;

		ConnectorThread(PlatformSocket* socket, float* timeout, float* retryFrequency);

	protected:
		State state;
		Host localHost;
		unsigned short localPort;

		void _updateConnecting();
		void _updateDisconnecting();
		void _updateProcess();

	};

}
#endif
