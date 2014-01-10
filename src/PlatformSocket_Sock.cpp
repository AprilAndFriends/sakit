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
#include <Iphlpapi.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
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
#include "Server.h"
#include "Socket.h"

#define CHAR_BUFFER 256

#if defined(_WIN32) && !defined(uint32_t)
typedef u_long uint32_t;
#endif

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

	PlatformSocket::PlatformSocket() : connectionLess(false)
	{
		this->connected = false;
		this->sock = -1;
		this->info = NULL;
		this->address = NULL;
		this->receiveBuffer = new char[bufferSize];
		memset(this->receiveBuffer, 0, bufferSize);
	}

	bool PlatformSocket::_setNonBlocking(bool value)
	{
		// set to blocking or non-blocking
		int setValue = (value ? 1 : 0);
		return this->_checkResult(ioctlsocket(this->sock, FIONBIO, (uint32_t*)&setValue), "ioctlsocket()");
	}

	bool PlatformSocket::createSocket(Ip host, unsigned short port)
	{
		this->connected = true;
		int result = 0;
		// create host info
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
#ifndef _ANDROID
		hints.ai_family = AF_INET;
#else
		hints.ai_family = PF_INET;
#endif
		hints.ai_socktype = (!this->connectionLess ? SOCK_STREAM : SOCK_DGRAM);
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
		if (!this->_checkResult(this->sock, "socket()"))
		{
			return false;
		}
		int reuseaddr = 1;
		return this->_checkResult(setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr)), "setsockopt()");
	}

	bool PlatformSocket::connect(Ip host, unsigned short port)
	{
		if (!this->createSocket(host, port))
		{
			return false;
		}
		// open connection
		return this->_checkResult(::connect(this->sock, this->info->ai_addr, this->info->ai_addrlen), "connect()");
	}

	bool PlatformSocket::bind(Ip host, unsigned short port)
	{
		if (!this->createSocket(host, port))
		{
			return false;
		}
		// bind to host:port
		return this->_checkResult(::bind(this->sock, this->info->ai_addr, this->info->ai_addrlen), "bind()");
	}

	bool PlatformSocket::_checkResult(int result, chstr functionName, bool disconnectOnError)
	{
		if (result == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError(functionName + " error!");
			if (disconnectOnError)
			{
				this->disconnect();
			}
			return false;
		}
		return true;
	}

	bool PlatformSocket::joinMulticastGroup(Ip host, unsigned short port, Ip groupAddress)
	{
		this->connected = true;
#ifndef _ANDROID
		int ai_family = AF_INET;
#else
		int ai_family = PF_INET;
#endif
		this->sock = socket(ai_family, SOCK_DGRAM, IPPROTO_UDP);
		if (!this->_checkResult(this->sock, "socket()"))
		{
			return false;
		}
		int reuseaddr = 1;
		if (!this->_checkResult(setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr)), "setsockopt()"))
		{
			return false;
		}
		sockaddr_in local;
		local.sin_family = ai_family;
		local.sin_port = htons(port);
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		if (!this->_checkResult(::bind(this->sock, (sockaddr*)&local, sizeof(sockaddr_in)), "bind()"))
		{
			return false;
		}
		ip_mreq group;
		group.imr_interface.s_addr = inet_addr(host.getAddress().c_str());
		group.imr_multiaddr.s_addr = inet_addr(groupAddress.getAddress().c_str());
		if (!this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(ip_mreq)), "setsockopt()"))
		{
			return false;
		}
		this->multicastGroupAddress.sin_family = ai_family;
		this->multicastGroupAddress.sin_addr.s_addr = inet_addr(groupAddress.getAddress().c_str());
		this->multicastGroupAddress.sin_port = htons(port);
		return this->setMulticastTtl(32);
	}

	bool PlatformSocket::setMulticastInterface(Ip address)
	{
		in_addr local;
		local.s_addr = inet_addr(address.getAddress().c_str());
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&local, sizeof(in_addr)), "setsockopt()");
	}

	bool PlatformSocket::setMulticastTtl(int value)
	{
		int ttl = 20;
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(int)), "setsockopt()");
	}

	bool PlatformSocket::setMulticastLoopback(bool value)
	{
		int loopBack = (value ? 1 : 0);
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopBack, sizeof(int)), "setsockopt()");
	}

	bool setMulticastTtl(int ttl);

	bool PlatformSocket::disconnect()
	{
		if (this->info != NULL)
		{
			freeaddrinfo(this->info);
			this->info = NULL;
		}
		if (this->address != NULL)
		{
			free(this->address);
			this->address = NULL;
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

	bool PlatformSocket::send(hstream* stream, int& sent, int& count)
	{
		const char* data = (const char*)&(*stream)[stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		int result = 0;
		if (!this->connectionLess)
		{
			result = ::send(this->sock, data, size, 0);
		}
		else if (this->info != NULL)
		{
			result = ::sendto(this->sock, data, size, 0, this->info->ai_addr, this->info->ai_addrlen);
		}
		else// if (this->address != NULL)
		{
			result = ::sendto(this->sock, data, size, 0, (sockaddr*)this->address, sizeof(*this->address));
		}
		/*
		else
		{
			result = ::sendto(this->sock, data, size, 0, (sockaddr*)&this->multicastGroupAddress, sizeof(this->multicastGroupAddress));
		}
		*/
		if (result >= 0)
		{
			stream->seek(result);
			sent += result;
			count -= result;
			return true;
		}
		return false;
	}

	bool PlatformSocket::receive(hstream* stream, hmutex& mutex, int& count)
	{
		interval.tv_sec = 5;
		interval.tv_usec = 0;
		memset(&this->readSet, 0, sizeof(this->readSet));
		unsigned long received = 0;
		// control socket IO
		if (!this->_checkResult(ioctlsocket(this->sock, FIONREAD, (uint32_t*)&received), "ioctlsocket()", false))
		{
			return false;
		}
		if (received == 0)
		{
			return true;
		}
		if (!this->_checkResult(recv(this->sock, this->receiveBuffer, hmin(count, bufferSize), 0), "recv()", false))
		{
			return false;
		}
		mutex.lock();
		stream->write_raw(this->receiveBuffer, received);
		mutex.unlock();
		count -= received;
		return true;
	}

	bool PlatformSocket::listen()
	{
		return this->_checkResult(::listen(this->sock, SOMAXCONN), "listen()", false);
	}

	bool PlatformSocket::accept(Socket* socket)
	{
		PlatformSocket* other = socket->socket;
		int size = (int)sizeof(sockaddr_storage);
		other->address = (sockaddr_storage*)malloc(size);
		this->_setNonBlocking(true);
		other->sock = ::accept(this->sock, (sockaddr*)other->address, &size);
		if (!other->_checkResult(other->sock, "accept()"))
		{
			this->_setNonBlocking(false);
			return false;
		}
		this->_setNonBlocking(false);
		// get the IP and port of the connected client
		char hostString[CHAR_BUFFER] = {'\0'};
		char portString[CHAR_BUFFER] = {'\0'};
		getnameinfo((sockaddr*)other->address, size, hostString, CHAR_BUFFER, portString, CHAR_BUFFER, NI_NUMERICHOST);
		((Base*)socket)->_activateConnection(Ip(hostString), (unsigned short)(int)hstr(portString));
		other->connected = true;
		return true;
	}

	bool PlatformSocket::receiveFrom(hstream* stream, Socket* socket)
	{
		PlatformSocket* other = socket->socket;
		int size = (int)sizeof(sockaddr_storage);
		other->address = (sockaddr_storage*)malloc(size);
		this->_setNonBlocking(true);
		int received = recvfrom(this->sock, this->receiveBuffer, bufferSize, 0, (sockaddr*)other->address, &size);
		if (!other->_checkResult(received, "recvfrom()"))
		{
			this->_setNonBlocking(false);
			return false;
		}
		this->_setNonBlocking(false);
		if (received > 0)
		{
			stream->write_raw(this->receiveBuffer, received);
			// get the IP and port of the connected client
			char hostString[CHAR_BUFFER] = {'\0'};
			char portString[CHAR_BUFFER] = {'\0'};
			getnameinfo((sockaddr*)other->address, size, hostString, CHAR_BUFFER, portString, CHAR_BUFFER, NI_NUMERICHOST);
			((Base*)socket)->_activateConnection(Ip(hostString), (unsigned short)(int)hstr(portString));
		}
		return true;
	}

	bool PlatformSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count)
	{
		const char* data = (const char*)&(*stream)[stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		int result = 0;
#ifndef _ANDROID
		int ai_family = AF_INET;
#else
		int ai_family = PF_INET;
#endif
		SOCKET sock = socket(ai_family, SOCK_DGRAM, IPPROTO_IP);
		if (sock == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError("socket() error!");
			closesocket(sock);
			return false;
		}
		int broadcast = 1;
		result = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast));
		if (result == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError("setsockopt() error!");
			closesocket(sock);
			return false;
		}
		int reuseaddress = 1;
		result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddress, sizeof(reuseaddress));
		if (result == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError("setsockopt() error!");
			closesocket(sock);
			return false;
		}
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
		address.sin_family = ai_family;
		address.sin_port = htons(port);
		int maxResult = 0;
		foreach (NetworkAdapter, it, adapters)
		{
			address.sin_addr.s_addr = inet_addr((*it).getBroadcastIp().getAddress().c_str());
			result = sendto(sock, data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));
			if (result == SOCKET_ERROR)
			{
				PlatformSocket::_printLastError("sendto() error!");
			}
			else if (result > 0)
			{
				maxResult = hmax(result, maxResult);
			}
		}
		closesocket(sock);
		if (maxResult > 0)
		{
			stream->seek(maxResult);
			return true;
		}
		return false;
	}

	harray<NetworkAdapter> PlatformSocket::getNetworkAdapters()
	{
		harray<NetworkAdapter> result;
#ifdef _WIN32
		PIP_ADAPTER_INFO info = NULL;
		unsigned long size = sizeof(IP_ADAPTER_INFO);
		info = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
		if (info == NULL)
		{
			hlog::error(sakit::logTag, "Not enough memory!");
			return result;
		}
		if (GetAdaptersInfo(info, &size) == ERROR_BUFFER_OVERFLOW) // gets the size required if necessary
		{
			free(info);
			info = (IP_ADAPTER_INFO*)malloc(size);
			if (info == NULL)
			{
				hlog::error(sakit::logTag, "Not enough memory!");
				return result;
			}
		}
		if ((GetAdaptersInfo(info, &size)) != NO_ERROR)
		{
			hlog::error(sakit::logTag, "Not enough memory!");
			return result;
		}
		int comboIndex;
		int index;
		hstr name;
		hstr description;
		hstr type;
		Ip address;
		Ip mask;
		Ip gateway;
		PIP_ADAPTER_INFO pAdapter = info;
        while (pAdapter != NULL)
		{
			comboIndex = pAdapter->ComboIndex;
			index = pAdapter->Index;
			name = pAdapter->AdapterName;
			description = pAdapter->Description;
            switch (pAdapter->Type)
			{
            case MIB_IF_TYPE_ETHERNET:
                type = "Ethernet";
                break;
            case MIB_IF_TYPE_TOKENRING:
                type = "Token Ring";
                break;
            case MIB_IF_TYPE_FDDI:
                type = "FDDI";
                break;
            case MIB_IF_TYPE_PPP:
                type = "PPP";
                break;
            case MIB_IF_TYPE_LOOPBACK:
                type = "Lookback";
                break;
            case MIB_IF_TYPE_SLIP:
                type = "Slip";
                break;
            case MIB_IF_TYPE_OTHER:
                type = "Other";
                break;
            default:
                type = "Unknown type " + hstr(pAdapter->Type);
                break;
            }
			address = Ip(pAdapter->IpAddressList.IpAddress.String);
			mask = Ip(pAdapter->IpAddressList.IpMask.String);
			gateway = Ip(pAdapter->GatewayList.IpAddress.String);
			result += NetworkAdapter(comboIndex, index, name, description, type, address, mask, gateway);
            pAdapter = pAdapter->Next;
		}
#else
		// TODOsock - implement Unix variant
		/*
		struct ifaddrs* ifaddr;
		struct ifaddrs* ifa;
        int family;
		int s;
		char host[NI_MAXHOST];

		if (getifaddrs(&ifaddr) == -1) {
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

		// Walk through linked list, maintaining head pointer so we can free list later

		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL)
				continue;

			family = ifa->ifa_addr->sa_family;

			// Display interface name and family (including symbolic form of the latter for the common families)

			printf("%s  address family: %d%s\n",
					ifa->ifa_name, family,
					(family == AF_PACKET) ? " (AF_PACKET)" :
					(family == AF_INET) ?   " (AF_INET)" :
					(family == AF_INET6) ?  " (AF_INET6)" : "");

			// For an AF_INET* interface address, display the address

			if (family == AF_INET || family == AF_INET6) {
				s = getnameinfo(ifa->ifa_addr,
						(family == AF_INET) ? sizeof(struct sockaddr_in) :
												sizeof(struct sockaddr_in6),
						host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (s != 0) {
					printf("getnameinfo() failed: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}
				printf("\taddress: <%s>\n", host);
			}
		}

		freeifaddrs(ifaddr);
		exit(EXIT_SUCCESS);
		*/
#endif
		return result;
	}

}
#endif
