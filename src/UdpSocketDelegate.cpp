/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "UdpSocketDelegate.h"

namespace sakit
{
	UdpSocketDelegate::UdpSocketDelegate() : SocketDelegate()
	{
	}

	UdpSocketDelegate::~UdpSocketDelegate()
	{
	}

	void UdpSocketDelegate::onReceived(Socket* socket, Host host, unsigned short port, hstream* stream)
	{
	}

}
