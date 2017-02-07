/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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

		bool connect(Host remoteHost, unsigned short remotePort);
		bool disconnect();

		bool connectAsync(Host remoteHost, unsigned short remotePort);
		bool disconnectAsync();

	protected:
		Connector(PlatformSocket* socket, ConnectorDelegate* connectorDelegate);

		void _integrate(State* stateValue, hmutex* mutexStateValue, Host* remoteHost, unsigned short* remotePort, Host* localHost, unsigned short* localPort, float* timeout, float* retryFrequency);
		void _update(float timeDelta = 0.0f);

		bool _canConnect(State state);
		bool _canDisconnect(State state);

	private:
		Connector(const Connector& other); // prevents copying

		PlatformSocket* _socket;
		State* _state;
		hmutex* _mutexState;
		Host* _remoteHost;
		unsigned short* _remotePort;
		Host* _localHost;
		unsigned short* _localPort;
		float* _timeout;
		float* _retryFrequency;
		ConnectorThread* _thread;
		ConnectorDelegate* _connectorDelegate;

	};

}
#endif
