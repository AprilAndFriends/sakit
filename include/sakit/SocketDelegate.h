/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a socket delegate.

#ifndef SAKIT_SOCKET_DELEGATE_H
#define SAKIT_SOCKET_DELEGATE_H

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class Socket;

	class sakitExport SocketDelegate
	{
	public:
		SocketDelegate();
		virtual ~SocketDelegate();

		virtual void onSent(Socket* socket, int byteCount);
		virtual void onSendFinished(Socket* socket);
		virtual void onSendFailed(Socket* socket);

		virtual void onReceiveFinished(Socket* socket);

	};

}
#endif
