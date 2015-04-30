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

		void update(float timeDelta = 0.0f);

		TcpSocket* accept();

	protected:
		harray<TcpSocket*> sockets;
		TcpServerThread* tcpServerThread;
		TcpServerDelegate* tcpServerDelegate;
		TcpSocketDelegate* acceptedDelegate;

		void _updateSockets();

	private:
		TcpServer(const TcpServer& other); // prevents copying

	};

}
#endif
