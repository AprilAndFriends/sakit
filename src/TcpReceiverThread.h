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
/// Defines a thread for receiving data through TCP.

#ifndef SAKIT_TCP_RECEIVER_THREAD_H
#define SAKIT_TCP_RECEIVER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "ReceiverThread.h"

namespace sakit
{
	class PlatformSocket;
	class TcpSocket;

	class TcpReceiverThread : public ReceiverThread
	{
	public:
		friend class TcpSocket;

		TcpReceiverThread(PlatformSocket* socket);
		~TcpReceiverThread();

	protected:
		hstream* stream;

		void _updateProcess();

	};

}
#endif
