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

#define SAKIT_HTTP_LINE_ENDING "\r\n"
#define SAKIT_HTTP_REQUEST_OPTIONS "OPTIONS"
#define SAKIT_HTTP_REQUEST_GET "GET"
#define SAKIT_HTTP_REQUEST_HEAD "HEAD"
#define SAKIT_HTTP_REQUEST_POST "POST"
#define SAKIT_HTTP_REQUEST_PUT "PUT"
#define SAKIT_HTTP_REQUEST_DELETE "DELETE"
#define SAKIT_HTTP_REQUEST_TRACE "TRACE"
#define SAKIT_HTTP_REQUEST_CONNECT "CONNECT"

namespace sakit
{
	class HttpResponse;
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
		/// @note This is due to keepAlive which has to be set beforehand
		bool isConnected();

		bool executeOptions(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeGet(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeHead(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executePost(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executePut(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeDelete(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeTrace(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeConnect(HttpResponse* response, Url url, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());

		// these are used when a persistent connection is available
		bool executeOptions(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeGet(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeHead(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executePost(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executePut(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeDelete(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeTrace(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());
		bool executeConnect(HttpResponse* response, hmap<hstr, hstr> customHeaders = hmap<hstr, hstr>());

		static unsigned short DefaultPort;

	protected:
		HttpSocketDelegate* socketDelegate;
		Protocol protocol;
		bool keepAlive;
		Url url;

		bool _executeMethod(HttpResponse* response, chstr method, Url& url, hmap<hstr, hstr>& customHeaders);
		bool _executeMethod(HttpResponse* response, chstr method, hmap<hstr, hstr>& customHeaders);
		bool _executeMethodInternal(HttpResponse* response, chstr method, Url& url, hmap<hstr, hstr>& customHeaders);

		int _send(hstream* stream, int count);
		bool _sendAsync(hstream* stream, int count);
		void _terminateConnection();

		int _receiveHttpDirect(HttpResponse* response);

		void _updateSending();
		void _updateReceiving();

		hstr _makeRequest(chstr method, Url url, hmap<hstr, hstr> customHeaders);
		hstr _makeProtocol();

	};

}
#endif
