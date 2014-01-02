/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WIN32) && !defined(_WINRT)
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
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

#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Ip.h"
#include "PlatformSocket.h"
#include "sakit.h"

namespace sakit
{
	extern int bufferSize;

	static timeval interval = {5, 0};

	void PlatformSocket::platformInit()
	{
#if defined(_WIN32) && !defined(_WINRT)
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0)
		{
			hlog::error(sakit::logTag, "Error: " + hstr(result));
		}
#endif
	}

	void PlatformSocket::platformDestroy()
	{
#if defined(_WIN32) && !defined(_WINRT)
		int result = WSACleanup();
		if (result != 0)
		{
			hlog::error(sakit::logTag, "Error: " + hstr(result));
		}
#endif
	}

	PlatformSocket::PlatformSocket()
	{
		this->connected = false;
		this->sock = -1;
		this->info = NULL;
		this->sendBuffer = new char[bufferSize];
		memset(this->sendBuffer, 0, bufferSize);
		this->receiveBuffer = new char[bufferSize];
		memset(this->receiveBuffer, 0, bufferSize);
	}

	bool PlatformSocket::_createSocket(Ip host, unsigned int port)
	{
		int result = 0;
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
		result = getaddrinfo(host.getAddress().split("/", 1).first().c_str(), hstr(port).c_str(), &hints, &this->info);
		if (result != 0)
		{
			hlog::error(sakit::logTag, hstr::from_unicode(gai_strerrorW(result)));
			this->disconnect();
			return false;
		}
		// create socket
		this->sock = socket(this->info->ai_family, this->info->ai_socktype, this->info->ai_protocol);
		if (this->sock == SOCKET_ERROR)
		{
			hlog::debug(sakit::logTag, "socket() error!");
			this->_printLastError();
			this->disconnect();
			return false;
		}
		return true;
	}

	bool PlatformSocket::connect(Ip host, unsigned int port)
	{
		if (!this->_createSocket(host, port))
		{
			return false;
		}
		// open connection
		return this->_finishSocket(::connect(this->sock, this->info->ai_addr, this->info->ai_addrlen), "connect()");
	}

	bool PlatformSocket::bind(Ip host, unsigned int port)
	{
		if (!this->_createSocket(host, port))
		{
			return false;
		}
		// bind to host:port
		return this->_finishSocket(::bind(this->sock, this->info->ai_addr, this->info->ai_addrlen), "bind()");
	}

	bool PlatformSocket::_finishSocket(int result, chstr functionName)
	{
		if (result == SOCKET_ERROR)
		{
			hlog::debug(sakit::logTag, functionName + " error!");
			this->_printLastError();
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
			this->sock = -1;
		}
		bool previouslyConnected = this->connected;
		this->connected = false;
		return previouslyConnected;
	}

	bool PlatformSocket::receive(hsbase* stream, hmutex& mutex, int& maxBytes)
	{
		interval.tv_sec = 5;
		interval.tv_usec = 0;
		memset(&this->readSet, 0, sizeof(this->readSet));
		unsigned long received = 0;
#ifdef _WIN32
		u_long* read = (u_long*)&received;
#else
		uint32_t* read = (uint32_t*)&received;
#endif
		int result = 0;
		// TODOsock - this will require most likely thread-safe access with mutexes, because of select and multiple TCP servers
		/*
		// select socket
		FD_ZERO(&this->readSet);
		FD_SET(this->sock, &this->readSet);
		result = select(0, &this->readSet, NULL, NULL, &interval);
		if (result == 0)
		{
			hlog::error(sakit::logTag, "Socket select timeout!");
			break;
		}
		if (result == SOCKET_ERROR)
		{
			hlog::debug(sakit::logTag, "select() error!");
			this->_printLastError();
			break;
		}
		//*/
		// control socket IO
		result = ioctlsocket(this->sock, FIONREAD, read);
		if (result == SOCKET_ERROR)
		{
			hlog::debug(sakit::logTag, "ioctlsocket() error!");
			this->_printLastError();
			return false;
		}
		if (received == 0)
		{
			return true;
		}
		received = recv(this->sock, this->receiveBuffer, hmin(maxBytes, bufferSize), 0);
		if (received == SOCKET_ERROR)
		{
			hlog::debug(sakit::logTag, "recv() error!");
			this->_printLastError();
			return false;
		}
		mutex.lock();
		stream->write_raw(this->receiveBuffer, received);
		mutex.unlock();
		maxBytes -= received;
		return true;
	}

	bool PlatformSocket::send(hsbase* stream, int& sent)
	{
		int size = hmin((int)(stream->size() - stream->position()), bufferSize);
		stream->read_raw(this->sendBuffer, size);
		int result = ::send(this->sock, this->sendBuffer, size, 0);
		if (result >= 0)
		{
			sent += result;
			return true;
		}
		return false;
	}

}
#endif
