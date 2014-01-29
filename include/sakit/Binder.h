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
/// Defines a binder fragment.

#ifndef SAKIT_BINDER_H
#define SAKIT_BINDER_H

#include <hltypes/hmutex.h>

#include "sakitExport.h"
#include "State.h"

namespace sakit
{
	class BinderDelegate;
	class BinderThread;
	class Host;
	class PlatformSocket;

	class sakitExport Binder
	{
	public:
		virtual ~Binder();

		bool isBinding();
		bool isBound();
		bool isUnbinding();

		// TODOsock - make it work with chstr port as well
		bool bind(Host host, unsigned short port);
		bool unbind();

		// TODOsock - make it work with chstr port as well
		bool bindAsync(Host host, unsigned short port);
		bool unbindAsync();

	protected:
		Binder(PlatformSocket* socket, BinderDelegate* binderDelegate);

		void _integrate(State* stateValue, hmutex* mutexStateValue, Host* host, unsigned short* port);
		void _update(float timeSinceLastFrame = 0.0f);

		bool _canBind(State state);
		bool _canUnbind(State state);

	private:
		PlatformSocket* _socket;
		State* _state;
		hmutex* _mutexState;
		Host* _host;
		unsigned short* _port;
		BinderThread* _thread;
		BinderDelegate* _binderDelegate;

	};

}
#endif
