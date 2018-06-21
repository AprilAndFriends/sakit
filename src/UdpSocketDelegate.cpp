/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "UdpSocketDelegate.h"

namespace sakit
{
	UdpSocketDelegate::UdpSocketDelegate() :
		SocketDelegate(),
		BinderDelegate()
	{
	}

	UdpSocketDelegate::~UdpSocketDelegate()
	{
	}

	void UdpSocketDelegate::onReceived(UdpSocket* socket, Host remoteHost, unsigned short remotePort, hstream* stream)
	{
	}

	void UdpSocketDelegate::onBroadcastFinished(UdpSocket* socket)
	{
	}

	void UdpSocketDelegate::onBroadcastFailed(UdpSocket* socket)
	{
	}

}
