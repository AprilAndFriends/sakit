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
/// Defines a special web based socket for HTTP operations.

#ifndef SAKIT_WEB_SOCKET_H
#define SAKIT_WEB_SOCKET_H

#include "sakitExport.h"
#include "TcpSocket.h"

namespace sakit
{
	class sakitExport WebSocket : public TcpSocket
	{
	public:
		WebSocket(SocketDelegate* socketDelegate);
		~WebSocket();

	};

}
#endif
