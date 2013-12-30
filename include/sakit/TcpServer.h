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
#include "TcpSocket.h"

namespace sakit
{
	class sakitExport TcpServer : public TcpSocket
	{
	public:
		TcpServer(ReceiverDelegate* receiverDelegate);
		~TcpServer();

	};

}
#endif
