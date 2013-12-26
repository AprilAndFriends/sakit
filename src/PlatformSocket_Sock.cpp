/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WIN32) && !defined(_WINRT)
#define _NO_WIN_H
#include <hltypes/hplatform.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x602
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

extern int h_errno;

#define SOCKET unsigned int
#define ioctlsocket ioctl
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR

#define socketret_t ssize_t
#define opt_t int
#define optlen_t socklen_t
#define CONNECT_WOULD_BLOCK EINPROGRESS
#define EWOULDBLOCK EAGAIN
#endif

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

namespace sakit
{
	static timeval interval = {5, 0};

	void PlatformSocket::platformInit()
	{
#if defined(_WIN32) && !defined(_WINRT)
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	}

	void PlatformSocket::platformDestroy()
	{
#if defined(_WIN32) && !defined(_WINRT)
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	}

	PlatformSocket::PlatformSocket()
	{
		this->connected = false;
		this->sock = 0;
		this->info = NULL;
		memset(this->buffer, 0, sizeof(BUFFER_SIZE));
	}

	bool PlatformSocket::connect(chstr host, unsigned int port)
	{
		// create host info
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
#ifndef _ANDROID
		hints.ai_family = AF_INET;
#else
		hints.ai_family = PF_INET;
#endif
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_IP;
		if (getaddrinfo(host.split("/", 1).first().c_str(), hstr(port).c_str(), &hints, &this->info) != 0)
		{
			// TODOsock - error message
			this->disconnect();
			return false;
		}
		// create socket
		this->sock = socket(this->info->ai_family, this->info->ai_socktype, this->info->ai_protocol);
		if (this->sock == -1)
		{
			// TODOsock - error message
			this->disconnect();
			return false;
		}
		// open socket
		if (::connect(this->sock, this->info->ai_addr, this->info->ai_addrlen) != 0)
		{
			// TODOsock - error message
			this->disconnect();
			return false;
		}
		this->connected = true;
		return true;
	}

	bool PlatformSocket::disconnect()
	{
		if (this->info != NULL)
		{
			freeaddrinfo(this->info);
			this->info = NULL;
		}
		if (this->sock != -1)
		{
			closesocket(this->sock);
			this->sock = 0;
		}
		bool previouslyConnected = this->connected;
		this->connected = false;
		return previouslyConnected;
	}

	void PlatformSocket::receive(hsbase& stream, int maxBytes)
	{
		interval.tv_sec = 5;
		interval.tv_usec = 0;
		memset(&this->readSet, 0, sizeof(this->readSet));
		int received = 0;
#ifdef _WIN32
		u_long* read = (u_long*)received;
#else
		uint32_t* read = (uint32_t*)received;
#endif
		while (true)
		{
			FD_ZERO(&this->readSet);
			FD_SET(this->sock, &this->readSet);
			if (select(this->sock + 1, &this->readSet, NULL, NULL, &interval) <= 0)
			{
				break;
			}
			if (ioctlsocket(this->sock, FIONREAD, read) == SOCKET_ERROR)
			{
				break;
			}
			if (received == 0)
			{
				break;
			}
			int received = recv(this->sock, this->buffer, hmin(maxBytes, BUFFER_SIZE), 0);
			if (received == SOCKET_ERROR || received < 0)
			{
				break;
			}
			stream.write_raw(this->buffer, received);
			maxBytes -= received;
			if (maxBytes == 0)
			{
				break;
			}
		}
	}
	
}
#endif
