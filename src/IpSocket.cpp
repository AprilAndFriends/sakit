/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "IpSocket.h"

namespace sakit
{
	IpSocket::IpSocket(ReceiverDelegate* receiverDelegate) : Socket(receiverDelegate)
	{
	}

	IpSocket::~IpSocket()
	{
	}
	
}
