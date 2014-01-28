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

		virtual void onSent(Socket* socket, int byteCount) = 0;
		virtual void onSendFinished(Socket* socket) = 0;
		virtual void onSendFailed(Socket* socket) = 0;

		virtual void onReceiveFinished(Socket* socket) = 0;

	};

}
#endif
