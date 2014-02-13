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
/// Defines a thread for executing a HTTP request.

#ifndef SAKIT_HTTP_SOCKET_THREAD_H
#define SAKIT_HTTP_SOCKET_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>

#include "Socket.h"
#include "TimedThread.h"

namespace sakit
{
	class PlatformSocket;
	class HttpResponse;
	class HttpSocket;

	class HttpSocketThread : public TimedThread
	{
	public:
		friend class HttpSocket;

		HttpSocketThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~HttpSocketThread();

	protected:
		hstream* stream;
		HttpResponse* response;

		void _updateConnect();
		void _updateSend();
		void _updateReceive();
		void _updateProcess();

	};

}
#endif
