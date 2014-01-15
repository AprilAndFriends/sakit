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

#include "SocketBase.h"
#include "sakitExport.h"
#include "TcpSocket.h"
#include "Url.h"

namespace sakit
{
	class HttpSocketDelegate;
	class PlatformSocket;
	class TcpSocket;

	class sakitExport HttpSocket : public SocketBase
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
		HL_DEFINE_SET(unsigned short, port, Port);

		bool executeGet(hstream* stream, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executePost(hstream* stream, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());

		static unsigned short DefaultPort;

	protected:
		HttpSocketDelegate* socketDelegate;
		Protocol protocol;
		bool keepAlive;

		bool _executeMethod(hstream* stream, chstr method, Url url, hmap<hstr, hstr>& customHeaders);

		int _send(hstream* stream, int count);
		bool _sendAsync(hstream* stream, int count);

		void _updateSending();
		void _updateReceiving();

		hstr _makeRequest(chstr method, Url url, hmap<hstr, hstr> customHeaders);
		hstr _makeProtocol();

	};

}
#endif
