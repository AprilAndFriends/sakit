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
/// Defines a special web based socket for HTTP operations.

#ifndef SAKIT_WEB_SOCKET_H
#define SAKIT_WEB_SOCKET_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"
#include "TcpSocket.h"

namespace sakit
{
	class sakitExport WebSocket : public TcpSocket
	{
	public:
		enum HttpProtocol
		{
			HTTP10,
			HTTP11,
			HTTP20
		};

		WebSocket(SocketDelegate* socketDelegate);
		~WebSocket();

		HL_DEFINE_GETSET(unsigned short, port, Port);
		HL_DEFINE_GETSET(HttpProtocol, protocol, Protocol);
		HL_DEFINE_GETSET2(hmap, hstr, hstr, headers, Headers);

		bool connect(Ip host);
		bool connectAsync(Ip host);

		int get(chstr url);
		int post(chstr url, hmap<hstr, hstr> parameters);

	protected:
		unsigned short port;
		HttpProtocol protocol;
		hmap<hstr, hstr> headers;

		hstr _makeUrl(chstr url);
		hstr _makeProtocol();
		hstr _makeHeaders();

	};

}
#endif
