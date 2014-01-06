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
/// Defines a server delegate.

#ifndef SAKIT_SERVER_DELEGATE_H
#define SAKIT_SERVER_DELEGATE_H

#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class Server;
	class Socket;

	class sakitExport ServerDelegate
	{
	public:
		ServerDelegate();
		virtual ~ServerDelegate();

		virtual void onBound(Server* server) = 0;
		virtual void onBindFailed(Server* server, Ip host, unsigned short port) = 0;
		virtual void onUnbound(Server* server, Ip host, unsigned short port) = 0;
		virtual void onUnbindFailed(Server* server) = 0;
		virtual void onStopped(Server* server) = 0;
		virtual void onRunFailed(Server* server) = 0;
		virtual void onAccepted(Server* server, Socket* socket) = 0;

	};

}
#endif
