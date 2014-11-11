/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a thread for binding a socket to a host and/or port.

#ifndef SAKIT_BINDER_THREAD_H
#define SAKIT_BINDER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>

#include "Server.h"
#include "State.h"
#include "WorkerThread.h"

namespace sakit
{
	class Binder;
	class PlatformSocket;

	class BinderThread : public WorkerThread
	{
	public:
		friend class Binder;

		BinderThread(PlatformSocket* socket);
		~BinderThread();

	protected:
		State state;

		void _updateBinding();
		void _updateUnbinding();
		void _updateProcess();

	};

}
#endif
