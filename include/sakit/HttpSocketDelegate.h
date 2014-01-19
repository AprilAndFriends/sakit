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
/// Defines a HTTP socket delegate.

#ifndef SAKIT_HTTP_SOCKET_DELEGATE_H
#define SAKIT_HTTP_SOCKET_DELEGATE_H

#include <hltypes/hstream.h>

#include "sakitExport.h"
#include "ServerDelegate.h"

namespace sakit
{
	class HttpSocket;

	class sakitExport HttpSocketDelegate
	{
	public:
		HttpSocketDelegate();
		virtual ~HttpSocketDelegate();

		virtual void onExecuteCompleted(HttpSocket* socket, HttpResponse* response, Url url) = 0;
		virtual void onExecuteFailed(HttpSocket* socket, HttpResponse* response, Url url) = 0;

	};

}
#endif
