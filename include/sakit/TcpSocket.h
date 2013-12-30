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
/// Defines a socket working over TCP.

#ifndef SAKIT_TCP_SOCKET_H
#define SAKIT_TCP_SOCKET_H

#include "sakitExport.h"
#include "IpSocket.h"

namespace sakit
{
	class sakitExport TcpSocket : public IpSocket
	{
	public:
		TcpSocket(ReceiverDelegate* receiverDelegate);
		~TcpSocket();

	};

}
#endif
