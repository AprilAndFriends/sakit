/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "UdpServerDelegate.h"

namespace sakit
{
	UdpServerDelegate::UdpServerDelegate()
	{
	}

	UdpServerDelegate::~UdpServerDelegate()
	{
	}

	void UdpServerDelegate::onReceived(UdpServer* server, Host remoetHost, unsigned short remotePort, hstream* stream)
	{
	}

}
