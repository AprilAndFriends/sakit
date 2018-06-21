/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "TcpSocketDelegate.h"

namespace sakit
{
	TcpSocketDelegate::TcpSocketDelegate() :
		SocketDelegate(),
		ConnectorDelegate()
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
