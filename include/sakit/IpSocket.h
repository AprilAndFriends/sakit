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
/// Defines a socket operating with IP addresses.

#ifndef SAKIT_IP_SOCKET_H
#define SAKIT_IP_SOCKET_H

#include "sakitExport.h"
#include "Socket.h"

namespace sakit
{
	class sakitExport IpSocket : public Socket
	{
	public:
		IpSocket();
		~IpSocket();

	};

}
#endif
