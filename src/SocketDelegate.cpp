/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "SocketDelegate.h"

namespace sakit
{
	SocketDelegate::SocketDelegate()
	{
	}

	SocketDelegate::~SocketDelegate()
	{
	}

	void SocketDelegate::onSent(Socket* socket, int byteCount)
	{
	}

	void SocketDelegate::onSendFinished(Socket* socket)
	{
	}

	void SocketDelegate::onSendFailed(Socket* socket)
	{
	}

	void SocketDelegate::onReceiveFinished(Socket* socket)
	{
	}

}
