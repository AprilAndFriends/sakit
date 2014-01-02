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
/// Defines a thread for sending data.

#ifndef SAKIT_SENDER_THREAD_H
#define SAKIT_SENDER_THREAD_H

#include <hltypes/hltypesUtil.h>

#include "sakitExport.h"
#include "WorkerThread.h"

namespace sakit
{
	class PlatformSocket;
	class Socket;

	class sakitExport SenderThread : public WorkerThread
	{
	public:
		friend class Socket;

		SenderThread(PlatformSocket* socket);
		~SenderThread();

	protected:
		int lastSent;

		void _updateProcess();

		static void process(hthread*);

	};

}
#endif
