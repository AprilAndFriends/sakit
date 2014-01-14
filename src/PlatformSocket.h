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
/// Defines a platform dependent implementation for socket functionality.

#ifndef SAKIT_PLATFORM_SOCKET_H
#define SAKIT_PLATFORM_SOCKET_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "Host.h"
#include "NetworkAdapter.h"
#include "sakitExport.h"

#ifdef __APPLE__
#include <netinet/in.h>
#endif

namespace sakit
{
	class Socket;

	class sakitExport PlatformSocket
	{
	public:
		PlatformSocket();
		~PlatformSocket();

		HL_DEFINE_IS(connected, Connected);
		HL_DEFINE_ISSET(connectionLess, ConnectionLess);

		bool createSocket(Host host, unsigned short port);
		bool connect(Host host, unsigned short port);
		bool bind(Host host, unsigned short port);
		bool disconnect();
		bool send(hstream* stream, int& sent, int& count);
		bool receive(hstream* stream, hmutex& mutex, int& count);
		bool receiveFrom(hstream* stream, Socket* socket);
		bool listen();
		bool accept(Socket* socket);

		bool joinMulticastGroup(Host host, unsigned short port, Host groupAddress);
		bool setMulticastInterface(Host address);
		bool setMulticastTtl(int value);
		bool setMulticastLoopback(bool value);

		static bool broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count = INT_MAX);
		static hstr resolveHost(chstr domain);
		static hstr resolveIp(chstr ip);
		static harray<NetworkAdapter> getNetworkAdapters();
		
		static void platformInit();
		static void platformDestroy();

	protected:
		bool connected;
		bool connectionLess;
		char* receiveBuffer;
		fd_set readSet;

#if !defined(_WIN32) || !defined(_WINRT)
		unsigned int sock;
		struct addrinfo* info;
		struct sockaddr_storage* address;
		struct sockaddr_in multicastGroupAddress;
#else
		Windows::Networking::Sockets::StreamSocket^ sock;
		Windows::Networking::HostName^ hostName;
		bool _asyncProcessing;
		bool _asyncFinished;
		unsigned int _asyncSize;
		bool _awaitAsync();
#endif

		bool _checkResult(int result, chstr functionName, bool disconnectOnError = true);

		bool _setNonBlocking(bool value);

		static int _printLastError(chstr basicMessage);

	};

}
#endif
