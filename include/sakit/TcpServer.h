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
/// Defines a TCP server.

#ifndef SAKIT_TCP_SERVER_H
#define SAKIT_TCP_SERVER_H

#include "sakitExport.h"
#include "Server.h"

namespace sakit
{
	class TcpServerDelegate;
	class TcpServerThread;
	class TcpSocket;
	class TcpSocketDelegate;

	class sakitExport TcpServer : public Server
	{
	public:
		TcpServer(TcpServerDelegate* serverDelegate, TcpSocketDelegate* acceptedDelegate);
		~TcpServer();

		harray<TcpSocket*> getSockets();

		void update(float timeSinceLastFrame);

		TcpSocket* accept(float timeout = 0.0f);

	protected:
		harray<TcpSocket*> sockets;
		TcpServerThread* thread;
		TcpServerDelegate* serverDelegate;
		TcpSocketDelegate* acceptedDelegate;

		void _updateSockets();

	};

}
#endif
