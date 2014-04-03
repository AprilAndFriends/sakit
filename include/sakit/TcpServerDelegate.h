/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a TCP server delegate.

#ifndef SAKIT_TCP_SERVER_DELEGATE_H
#define SAKIT_TCP_SERVER_DELEGATE_H

#include "sakitExport.h"
#include "ServerDelegate.h"

namespace sakit
{
	class TcpServer;
	class TcpSocket;

	class sakitExport TcpServerDelegate : public ServerDelegate
	{
	public:
		TcpServerDelegate();
		~TcpServerDelegate();

		virtual void onAccepted(TcpServer* server, TcpSocket* socket);

	};

}
#endif
