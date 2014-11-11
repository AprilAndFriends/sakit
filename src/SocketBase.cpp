/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "sakit.h"
#include "SocketBase.h"

namespace sakit
{
	SocketBase::SocketBase() : Base(), remotePort(0)
	{
	}

	SocketBase::~SocketBase()
	{
	}

	int SocketBase::_send(chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->_send(&stream, stream.size());
	}

	void SocketBase::_activateConnection(Host remoteHost, unsigned short remotePort, Host localHost, unsigned short localPort)
	{
		this->remoteHost = remoteHost;
		this->remotePort = remotePort;
		this->localHost = localHost;
		this->localPort = localPort;
	}

}
