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
/// Defines a generic server.

#ifndef SAKIT_SERVER_H
#define SAKIT_SERVER_H

#include <hltypes/hltypesUtil.h>

#include "Base.h"
#include "Binder.h"
#include "Host.h"
#include "sakitExport.h"
#include "State.h"

namespace sakit
{
	class ServerDelegate;
	class WorkerThread;

	class sakitExport Server : public Base, public Binder
	{
	public:
		~Server();

		bool isRunning();

		void update(float timeSinceLastFrame = 0.0f);

		bool startAsync();
		bool stopAsync();

	protected:
		WorkerThread* serverThread;

		Server(ServerDelegate* serverDelegate);

		bool _canStart(State state);
		bool _canStop(State state);

	private:
		ServerDelegate* serverDelegate;

	};

}
#endif
