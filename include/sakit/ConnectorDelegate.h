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
/// Defines a connector delegate.

#ifndef SAKIT_CONNECTOR_DELEGATE_H
#define SAKIT_CONNECTOR_DELEGATE_H

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class Connector;

	class sakitExport ConnectorDelegate
	{
	public:
		ConnectorDelegate();
		virtual ~ConnectorDelegate();

		virtual void onConnected(Connector* connector, Host host, unsigned short port) = 0;
		virtual void onDisconnected(Connector* connector, Host host, unsigned short port) = 0;
		virtual void onConnectFailed(Connector* connector, Host host, unsigned short port) = 0;
		virtual void onDisconnectFailed(Connector* connector, Host host, unsigned short port) = 0;

	};

}
#endif
