/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstream.h>
#include <hltypes/harray.h>

#include "Host.h"
#include "HttpTcpSocketDelegate.h"
#include "PlatformSocket.h"
#include "Socket.h"

namespace sakit
{
	HttpTcpSocketDelegate::HttpTcpSocketDelegate() : SocketDelegate()
	{
	}

	HttpTcpSocketDelegate::~HttpTcpSocketDelegate()
	{
	}

	void HttpTcpSocketDelegate::onConnected(sakit::Socket* socket)
	{
	}

	void HttpTcpSocketDelegate::onDisconnected(sakit::Socket* socket, sakit::Host host, unsigned short port)
	{
	}

	void HttpTcpSocketDelegate::onConnectFailed(sakit::Socket* socket, sakit::Host host, unsigned short port)
	{
	}

	void HttpTcpSocketDelegate::onDisconnectFailed(sakit::Socket* socket)
	{
	}

	void HttpTcpSocketDelegate::onSent(sakit::Socket* socket, int bytes)
	{
	}

	void HttpTcpSocketDelegate::onSendFinished(sakit::Socket* socket)
	{
	}

	void HttpTcpSocketDelegate::onSendFailed(sakit::Socket* socket)
	{
	}

	void HttpTcpSocketDelegate::onReceived(sakit::Socket* socket, hstream* stream)
	{
	}

	void HttpTcpSocketDelegate::onReceiveFinished(sakit::Socket* socket)
	{
	}

	void HttpTcpSocketDelegate::onReceiveFailed(sakit::Socket* socket)
	{
	}

}
