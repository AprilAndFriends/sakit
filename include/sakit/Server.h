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
/// Defines a generic server.

#ifndef SAKIT_SERVER_H
#define SAKIT_SERVER_H

#include <hltypes/hltypesUtil.h>

#include "Base.h"
#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class ServerDelegate;

	class sakitExport Server : public Base
	{
	public:
		Server(ServerDelegate* serverDelegate);
		~Server();

		bool isBound();

		// TODOsock - allow chstr port as well?
		bool bind(Ip host, unsigned short port);
		bool unbind();

	protected:
		ServerDelegate* serverDelegate;

	};

}
#endif
