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
#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class ServerDelegate;
	class ServerThread;
	class Socket;
	class SocketDelegate;

	class sakitExport Server : public Base
	{
	public:
		enum State
		{
			IDLE,
			BINDING,
			BOUND,
			RUNNING,
			UNBINDING
		};

		Server(ServerDelegate* serverDelegate, SocketDelegate* acceptedDelegate);
		~Server();

		HL_DEFINE_GET(harray<Socket*>, sockets, Sockets);
		bool isBinding();
		bool isBound();
		bool isRunning();
		bool isUnbinding();

		bool bind(Ip host, unsigned short port);
		bool unbind();
		Socket* accept(float timeout);
		bool destroy(Socket* socket);

		bool bindAsync(Ip host, unsigned short port);
		bool unbindAsync();
		bool startAsync();
		bool stopAsync();

		void update(float timeSinceLastFrame);

	protected:
		ServerDelegate* serverDelegate;
		SocketDelegate* acceptedDelegate;
		ServerThread* thread;
		harray<Socket*> sockets;

		bool _checkBindStatus(State state);
		bool _checkStartStatus(State state);
		bool _checkStopStatus(State state);
		bool _checkUnbindStatus(State state);

	};

}
#endif
