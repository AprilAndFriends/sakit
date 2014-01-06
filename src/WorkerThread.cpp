/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "WorkerThread.h"

namespace sakit
{
	WorkerThread::WorkerThread(void (*function)(hthread*), PlatformSocket* socket) :
		hthread(function), result(IDLE), host(""), port(0)
	{
		this->socket = socket;
	}

	WorkerThread::~WorkerThread()
	{
	}

}
