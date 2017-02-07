/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "ConnectorDelegate.h"

namespace sakit
{
	ConnectorDelegate::ConnectorDelegate()
	{
	}

	ConnectorDelegate::~ConnectorDelegate()
	{
	}

	void ConnectorDelegate::onConnected(Connector* connector, Host remoteHost, unsigned short remotePort)
	{
	}

	void ConnectorDelegate::onDisconnected(Connector* connector, Host remoteHost, unsigned short remotePort)
	{
	}

	void ConnectorDelegate::onConnectFailed(Connector* connector, Host remoteHost, unsigned short remotePort)
	{
	}

	void ConnectorDelegate::onDisconnectFailed(Connector* connector, Host remoteHost, unsigned short remotePort)
	{
	}

}
