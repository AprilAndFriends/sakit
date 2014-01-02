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
/// Defines a thread for executing async socket work.

#ifndef SAKIT_WORKER_THREAD_H
#define SAKIT_WORKER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;
	
	class sakitExport WorkerThread : public hthread
	{
	public:
		enum State
		{
			IDLE,
			RUNNING,
			FINISHED,
			FAILED
		};

		WorkerThread(void (*function)(hthread*), PlatformSocket* socket);
		~WorkerThread();

	protected:
		State state;
		PlatformSocket* socket;
		hstream* stream;
		hmutex mutex;

	};

}
#endif
