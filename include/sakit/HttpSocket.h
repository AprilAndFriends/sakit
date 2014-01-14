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
/// Defines a special socket for HTTP communication.

#ifndef SAKIT_HTTP_SOCKET_H
#define SAKIT_HTTP_SOCKET_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"
#include "TcpSocket.h"
#include "Uri.h"

namespace sakit
{
	class HttpSocketDelegate;
	class HttpTcpSocketDelegate;
	class TcpSocket;

	class sakitExport HttpSocket
	{
	public:
		enum Protocol
		{
			HTTP11
		};

		HttpSocket(HttpSocketDelegate* socketDelegate, Protocol protocol = HTTP11);
		~HttpSocket();

		HL_DEFINE_ISSET(keepAlive, KeepAlive);
		HL_DEFINE_GETSET(Protocol, protocol, Protocol);
		HL_DEFINE_GETSET2(hmap, hstr, hstr, customHeaders, CustomHeaders);

		bool get(Uri uri);
		bool get(Host host, chstr path);
		bool post(chstr url, hmap<hstr, hstr> parameters);

	protected:
		TcpSocket* socket;
		HttpSocketDelegate* socketDelegate;
		SocketDelegate* tcpSocketDelegate;
		Protocol protocol;
		bool keepAlive;


		hmap<hstr, hstr> customHeaders;

		hstr _makeUrl(chstr url);
		hstr _makeProtocol();
		hstr _makeHeaders();

	};

}
#endif
