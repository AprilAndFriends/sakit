/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "TcpSocketDelegate.h"

namespace sakit
{
	TcpSocketDelegate::TcpSocketDelegate() : SocketDelegate(), ConnectorDelegate()
	{
	}

	TcpSocketDelegate::~TcpSocketDelegate()
	{
	}

	void TcpSocketDelegate::onReceived(TcpSocket* socket, hstream* stream)
	{
	}

	void TcpSocketDelegate::onReceiveFailed(TcpSocket* socket)
	{
	}

}
