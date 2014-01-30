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
/// Defines a connector fragment.

#ifndef SAKIT_CONNECTOR_H
#define SAKIT_CONNECTOR_H

#include <hltypes/hmutex.h>

#include "sakitExport.h"
#include "State.h"

namespace sakit
{
	class ConnectorDelegate;
	class ConnectorThread;
	class Host;
	class PlatformSocket;

	class sakitExport Connector
	{
	public:
		virtual ~Connector();

		bool isConnecting();
		bool isConnected();
		bool isDisconnecting();

		// TODOsock - make it work with chstr port as well
		bool connect(Host remoteHost, unsigned short remotePort);
		bool disconnect();

		// TODOsock - make it work with chstr port as well
		bool connectAsync(Host remoteHost, unsigned short remotePort);
		bool disconnectAsync();

	protected:
		Connector(PlatformSocket* socket, ConnectorDelegate* connectorDelegate);

		void _integrate(State* stateValue, hmutex* mutexStateValue, Host* remoteHost, unsigned short* remotePort, Host* localHost, unsigned short* localPort);
		void _update(float timeSinceLastFrame = 0.0f);

		bool _canConnect(State state);
		bool _canDisconnect(State state);

	private:
		PlatformSocket* _socket;
		State* _state;
		hmutex* _mutexState;
		Host* _remoteHost;
		unsigned short* _remotePort;
		Host* _localHost;
		unsigned short* _localPort;
		ConnectorThread* _thread;
		ConnectorDelegate* _connectorDelegate;

	};

}
#endif
