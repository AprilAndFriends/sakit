/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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
#include "SocketBase.h"
#include "State.h"
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
	class HttpSocketThread;
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
		HL_DEFINE_SET(unsigned short, remotePort, RemotePort);
		/// @note This is due to keepAlive which has to be set beforehand
		bool isConnected();
		bool isExecuting();

		void update(float timeDelta = 0.0f);

		bool executeOptions(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeGet(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeHead(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePost(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePut(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeDelete(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeTrace(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeConnect(HttpResponse* response, Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));

		bool executeOptionsAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeGetAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeHeadAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePostAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePutAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeDeleteAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeTraceAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeConnectAsync(Url url, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));

		// these are used when a persistent connection is available
		bool executeOptions(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeGet(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeHead(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePost(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePut(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeDelete(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeTrace(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeConnect(HttpResponse* response, chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));

		bool executeOptionsAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeGetAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeHeadAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePostAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executePutAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeDeleteAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeTraceAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));
		bool executeConnectAsync(chstr customBody = "", hmap<hstr, hstr> customHeaders = (hmap<hstr, hstr>()));

		static unsigned short DefaultPort;

	protected:
		HttpSocketDelegate* socketDelegate;
		HttpSocketThread* thread;
		Protocol protocol;
		bool keepAlive;
		Url url;

		bool _executeMethod(HttpResponse* response, chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders);
		bool _executeMethod(HttpResponse* response, chstr method, chstr customBody, hmap<hstr, hstr>& customHeaders);
		bool _executeMethodInternal(HttpResponse* response, chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders);

		bool _executeMethodAsync(chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders);
		bool _executeMethodAsync(chstr method, chstr customBody, hmap<hstr, hstr>& customHeaders);
		bool _executeMethodInternalAsync(chstr method, Url& url, chstr customBody, hmap<hstr, hstr>& customHeaders);

		int _send(hstream* stream, int count);
		bool _sendAsync(hstream* stream, int count);
		void _terminateConnection();

		int _receiveHttpDirect(HttpResponse* response);

		bool _canExecute(State state);

		hstr _processRequest(chstr method, Url url, chstr customBody, hmap<hstr, hstr> customHeaders);

		hstr _makeProtocol();

	};

}
#endif
