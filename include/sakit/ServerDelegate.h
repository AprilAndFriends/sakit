/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a server delegate.

#ifndef SAKIT_SERVER_DELEGATE_H
#define SAKIT_SERVER_DELEGATE_H

#include "BinderDelegate.h"
#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class Server;
	class Socket;

	class sakitExport ServerDelegate : public BinderDelegate
	{
	public:
		ServerDelegate();
		~ServerDelegate();

		virtual void onStopped(Server* server);
		virtual void onStartFailed(Server* server);

	};

}
#endif
