/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if !defined(_WIN32) || !defined(_WINRT)
#ifdef __APPLE__
#include <TargetConditionals.h>
#include <netinet/tcp.h>
#endif
#define _NO_WIN_H
#include <hltypes/hplatform.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x602

typedef int socklen_t;

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
#include <netinet/tcp.h>
#include <arpa/inet.h>
//#include <ifaddrs.h>
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

#include "Host.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Server.h"
#include "Socket.h"

#ifdef _WIN32
	#define __gai_strerror(x) hstr::from_unicode(gai_strerrorW(x))
#else
	#define __gai_strerror(x) hstr(gai_strerror(x))
#endif

#if defined(_WIN32) && !defined(uint32_t)
typedef u_long uint32_t;
#endif

namespace sakit
{
	extern int bufferSize;

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

	PlatformSocket::PlatformSocket() : connected(false), connectionLess(false)
	{
		this->sock = -1;
		this->socketInfo = NULL;
		this->localInfo = NULL;
		this->remoteInfo = NULL;
		this->address = NULL;
		this->bufferSize = sakit::bufferSize;
		this->receiveBuffer = new char[this->bufferSize];
		memset(this->receiveBuffer, 0, this->bufferSize);
	}

	bool PlatformSocket::_setNonBlocking(bool value)
	{
		// set to blocking or non-blocking
		int setValue = (value ? 1 : 0);
		return this->_checkResult(ioctlsocket(this->sock, FIONBIO, (uint32_t*)&setValue), "ioctlsocket()");
	}

	bool PlatformSocket::tryCreateSocket()
	{
		if (this->sock == -1)
		{
			this->connected = true;
			this->sock = socket(this->socketInfo->ai_family, this->socketInfo->ai_socktype, this->socketInfo->ai_protocol);
			if (!this->_checkResult(this->sock, "socket()"))
			{
				this->disconnect();
				return false;
			}
		}
		return true;
	}

	bool PlatformSocket::setRemoteAddress(Host remoteHost, unsigned short remotePort)
	{
		return this->_setAddress(remoteHost, remotePort, &this->remoteInfo);
	}

	bool PlatformSocket::setLocalAddress(Host localHost, unsigned short localPort)
	{
		return this->_setAddress(localHost, localPort, &this->localInfo);
	}

	bool PlatformSocket::_setAddress(Host& host, unsigned short& port, addrinfo** info)
	{
		if (this->socketInfo == NULL)
		{
			this->socketInfo = (addrinfo*)malloc(sizeof(addrinfo));
			memset(this->socketInfo, 0, sizeof(addrinfo));
		}
		if (*info != NULL)
		{
			freeaddrinfo(*info);
			*info = NULL;
		}
#ifndef _ANDROID
		this->socketInfo->ai_family = AF_INET;
#else
		this->socketInfo->ai_family = PF_INET;
#endif
		this->socketInfo->ai_socktype = (!this->connectionLess ? SOCK_STREAM : SOCK_DGRAM);
		this->socketInfo->ai_protocol = IPPROTO_IP;
		this->socketInfo->ai_flags = AI_NUMERICSERV;
		int result = getaddrinfo(host.toString().c_str(), hstr(port).c_str(), this->socketInfo, info);
		if (result != 0)
		{
			hlog::error(sakit::logTag, __gai_strerror(result));
			this->disconnect();
			return false;
		}
		this->socketInfo->ai_family = (*info)->ai_family;
		this->socketInfo->ai_socktype = (*info)->ai_socktype;
		this->socketInfo->ai_protocol = (*info)->ai_protocol;
		return true;
	}

	bool PlatformSocket::connect(Host remoteHost, unsigned short remotePort, Host& localHost, unsigned short& localPort)
	{
		if (!this->setRemoteAddress(remoteHost, remotePort))
		{
			return false;
		}
		if (!this->tryCreateSocket())
		{
			return false;
		}
		if (this->connectionLess)
		{
			return true;
		}
		if (!this->setNagleAlgorithmActive(false))
		{
			return false;
		}
		// open connection
		if (!this->_checkResult(::connect(this->sock, this->remoteInfo->ai_addr, this->remoteInfo->ai_addrlen), "connect()"))
		{
			return false;
		}
		this->_getLocalHostPort(localHost, localPort);
		return true;
	}

	bool PlatformSocket::bind(Host localHost, unsigned short& localPort)
	{
		if (!this->setLocalAddress(localHost, localPort))
		{
			return false;
		}
		if (!this->tryCreateSocket())
		{
			return false;
		}
		// bind to host:port
		if (!this->_checkResult(::bind(this->sock, this->localInfo->ai_addr, this->localInfo->ai_addrlen), "bind()"))
		{
			return false;
		}
		if (localPort == 0)
		{
			this->_getLocalHostPort(localHost, localPort);
		}
		return true;
	}

	void PlatformSocket::_getLocalHostPort(Host& host, unsigned short& port)
	{
		sockaddr_in address;
		socklen_t addressSize = (socklen_t)sizeof(sockaddr_in);
		memset(&address, 0, addressSize);
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(host.toString().c_str());
		address.sin_port = htons(port);
		getsockname(this->sock, (sockaddr*)&address, &addressSize);
		host = Host(inet_ntoa(address.sin_addr));
		port = ntohs(address.sin_port);
	}

	bool PlatformSocket::joinMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		ip_mreq group;
		group.imr_interface.s_addr = inet_addr(interfaceHost.toString().c_str());
		group.imr_multiaddr.s_addr = inet_addr(groupAddress.toString().c_str());
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(ip_mreq)), "setsockopt()");
	}

	bool PlatformSocket::leaveMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		ip_mreq group;
		group.imr_interface.s_addr = inet_addr(interfaceHost.toString().c_str());
		group.imr_multiaddr.s_addr = inet_addr(groupAddress.toString().c_str());
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&group, sizeof(ip_mreq)), "setsockopt()");
	}

	bool PlatformSocket::setNagleAlgorithmActive(bool value)
	{
		int noDelay = (value ? 0 : 1);
		return this->_checkResult(setsockopt(this->sock, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(int)), "setsockopt()");
	}

	bool PlatformSocket::setMulticastInterface(Host interfaceHost)
	{
		in_addr local;
		local.s_addr = inet_addr(interfaceHost.toString().c_str());
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&local, sizeof(in_addr)), "setsockopt()");
	}

	bool PlatformSocket::setMulticastTtl(int value)
	{
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&value, sizeof(int)), "setsockopt()");
	}

	bool PlatformSocket::setMulticastLoopback(bool value)
	{
		int loopBack = (value ? 1 : 0);
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopBack, sizeof(int)), "setsockopt()");
	}

	bool PlatformSocket::disconnect()
	{
		if (this->socketInfo != NULL)
		{
			free(this->socketInfo);
			this->socketInfo = NULL;
		}
		if (this->localInfo != NULL)
		{
			freeaddrinfo(this->localInfo);
			this->localInfo = NULL;
		}
		if (this->remoteInfo != NULL)
		{
			freeaddrinfo(this->remoteInfo);
			this->remoteInfo = NULL;
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

	bool PlatformSocket::send(hstream* stream, int& count, int& sent)
	{
		const char* data = (const char*)&(*stream)[stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		int result = 0;
		if (!this->connectionLess)
		{
			result = ::send(this->sock, data, size, 0);
		}
		else if (this->remoteInfo != NULL)
		{
			result = ::sendto(this->sock, data, size, 0, this->remoteInfo->ai_addr, this->remoteInfo->ai_addrlen);
		}
		else if (this->address != NULL)
		{
			result = ::sendto(this->sock, data, size, 0, (sockaddr*)this->address, sizeof(*this->address));
		}
		else
		{
			hlog::warn(sakit::logTag, "Trying to send without a remote host!");
		}
		if (result >= 0)
		{
			stream->seek(result);
			sent += result;
			count -= result;
			return true;
		}
		return false;
	}

	bool PlatformSocket::receive(hstream* stream, hmutex& mutex, int& maxBytes)
	{
		unsigned long received = 0;
		if (!this->_checkReceivedBytes(&received))
		{
			return false;
		}
		if (received == 0)
		{
			return true;
		}
		int read = hmin((int)received, this->bufferSize);
		if (maxBytes > 0) // if don't read everything
		{
			read = hmin(read, maxBytes);
		}
		read = recv(this->sock, this->receiveBuffer, read, 0);
		if (!this->_checkResult(read, "recv()", false))
		{
			return false;
		}
		mutex.lock();
		stream->write_raw(this->receiveBuffer, read);
		mutex.unlock();
		if (maxBytes > 0) // if don't read everything
		{
			maxBytes -= read;
		}
		return true;
	}

	bool PlatformSocket::receiveFrom(hstream* stream, Host& remoteHost, unsigned short& remotePort)
	{
		unsigned long received = 0;
		if (!this->_checkReceivedBytes(&received))
		{
			return false;
		}
		if (received == 0)
		{
			return true;
		}
		int read = hmin((int)received, this->bufferSize);
		sockaddr_storage address;
		socklen_t size = (socklen_t)sizeof(sockaddr_storage);
		this->_setNonBlocking(true);
		read = recvfrom(this->sock, this->receiveBuffer, read, 0, (sockaddr*)&address, &size);
		if (!this->_checkResult(read, "recvfrom()"))
		{
			this->_setNonBlocking(false);
			return false;
		}
		this->_setNonBlocking(false);
		if (read > 0)
		{
			stream->write_raw(this->receiveBuffer, read);
			// get the IP and port of the connected client
			char hostString[NI_MAXHOST] = {'\0'};
			char portString[NI_MAXSERV] = {'\0'};
			getnameinfo((sockaddr*)&address, size, hostString, NI_MAXHOST, portString, NI_MAXSERV, NI_NUMERICHOST);
			remoteHost = Host(hostString);
			remotePort = (unsigned short)(int)hstr(portString);
		}
		return true;
	}

	bool PlatformSocket::_checkReceivedBytes(unsigned long* received)
	{
#ifndef _WIN32 // Unix requires a select() call before using ioctl/ioctlsocket
		timeval interval = {1, 0};
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(this->sock, &readSet);
		int result = select(this->sock + 1, &readSet, NULL, NULL, &interval);
		if (!this->_checkResult(result, "select()") || result == 0)
		{
			return false;
		}
#endif
		// control socket IO
		return this->_checkResult(ioctlsocket(this->sock, FIONREAD, (uint32_t*)received), "ioctlsocket()", false);
	}

	bool PlatformSocket::listen()
	{
		return this->_checkResult(::listen(this->sock, SOMAXCONN), "listen()", false);
	}

	bool PlatformSocket::accept(Socket* socket)
	{
		PlatformSocket* other = socket->socket;
		socklen_t size = (socklen_t)sizeof(sockaddr_storage);
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
		char hostString[NI_MAXHOST] = {'\0'};
		char portString[NI_MAXSERV] = {'\0'};
		getnameinfo((sockaddr*)other->address, size, hostString, NI_MAXHOST, portString, NI_MAXSERV, NI_NUMERICHOST);
		Host localHost;
		unsigned short localPort = 0;
		this->_getLocalHostPort(localHost, localPort);
		((SocketBase*)socket)->_activateConnection(Host(hostString), (unsigned short)(int)hstr(portString), localHost, localPort);
		other->connected = true;
		return true;
	}

	bool PlatformSocket::_checkResult(int result, chstr functionName, bool disconnectOnError)
	{
		if (result == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError(functionName);
			if (disconnectOnError)
			{
				this->disconnect();
			}
			return false;
		}
		return true;
	}

	bool PlatformSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count)
	{
		const char* data = (const char*)&(*stream)[stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		int broadcast = 1;
		if (!this->_checkResult(setsockopt(this->sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)), "setsockopt", false))
		{
			return false;
		}
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
#ifndef _ANDROID
		address.sin_family = AF_INET;
#else
		address.sin_family = PF_INET;
#endif
		address.sin_port = htons(port);
		int result = 0;
		int maxResult = 0;
		socklen_t addrSize = sizeof(sockaddr_in);
		Host broadcastIp;
		harray<Host> ips;
		foreach (NetworkAdapter, it, adapters)
		{
			ips += (*it).getBroadcastIp();
		}
		ips.remove_duplicates(); // to avoid broadcasting on the same IP twice, just to be sure
		foreach (Host, it, ips)
		{
			address.sin_addr.s_addr = inet_addr((*it).toString().c_str());
			result = sendto(this->sock, data, size, 0, (sockaddr*)&address, addrSize);
			if (this->_checkResult(result, "sendto", false) && result > 0)
			{
				maxResult = hmax(result, maxResult);
			}
		}
		broadcast = 0;
		// this one must not fail or the socket will be in an unconsistent state
		if (!this->_checkResult(setsockopt(this->sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)), "setsockopt"))
		{
			return false;
		}
		if (maxResult > 0)
		{
			stream->seek(maxResult);
			return true;
		}
		return false;
	}

	Host PlatformSocket::resolveHost(Host domain)
	{
		addrinfo hints;
		addrinfo* info;
		memset(&hints, 0, sizeof(hints));
#ifndef _ANDROID
		hints.ai_family = AF_INET;
#else
		hints.ai_family = PF_INET;
#endif
		int result = getaddrinfo(domain.toString().c_str(), NULL, &hints, &info);
		if (result != 0)
		{
			hlog::error(sakit::logTag, __gai_strerror(result));
			return Host();
		}
		in_addr address;
		address.s_addr = ((sockaddr_in*)(info->ai_addr))->sin_addr.s_addr;
		freeaddrinfo(info);
		return Host(inet_ntoa(address));
	}

	Host PlatformSocket::resolveIp(Host ip)
	{
		sockaddr_in address;
#ifndef _ANDROID
		address.sin_family = AF_INET;
#else
		address.sin_family = PF_INET;
#endif
		inet_pton(address.sin_family, ip.toString().c_str(), &address.sin_addr);
		char hostName[NI_MAXHOST] = {'\0'};
		int result = getnameinfo((sockaddr*)&address, sizeof(address), hostName, sizeof(hostName), NULL, 0, NI_NUMERICHOST);
		if (result != 0)
		{
			hlog::error(sakit::logTag, __gai_strerror(result));
			return Host();
		}
		return Host(hostName);
	}

	unsigned short PlatformSocket::resolveServiceName(chstr serviceName)
	{
		addrinfo hints;
		addrinfo* info;
		memset(&hints, 0, sizeof(hints));
#ifndef _ANDROID
		hints.ai_family = AF_INET;
#else
		hints.ai_family = PF_INET;
#endif
		int result = getaddrinfo(NULL, serviceName.c_str(), &hints, &info);
		if (result != 0)
		{
			hlog::error(sakit::logTag, __gai_strerror(result));
			return 0;
		}
		unsigned short port = ((sockaddr_in*)(info->ai_addr))->sin_port;
		freeaddrinfo(info);
		return port;
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
		Host address;
		Host mask;
		Host gateway;
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
			address = Host(pAdapter->IpAddressList.IpAddress.String);
			mask = Host(pAdapter->IpAddressList.IpMask.String);
			gateway = Host(pAdapter->GatewayList.IpAddress.String);
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
		if (getifaddrs(&ifaddr) != -1)
		{
			for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr != NULL)
				{
					family = ifa->ifa_addr->sa_family;

					// Display interface name and family (including symbolic form of the latter for the common families)

					printf("%s  address family: %d%s\n",
							ifa->ifa_name, family,
							(family == AF_PACKET) ? " (AF_PACKET)" :
							(family == AF_INET) ?   " (AF_INET)" :
							(family == AF_INET6) ?  " (AF_INET6)" : "");

					// For an AF_INET* interface address, display the address

					if (family == AF_INET || family == AF_INET6)
					{
						s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
						if (s == 0)
						{
							//printf("\taddress: <%s>\n", host);
						}
					}
				}
			}
			freeifaddrs(ifaddr);
		}
		*/
		// TODOsock - or this one
		/*
		struct ifaddrs* ifAddrStruct = NULL;
		struct ifaddrs* ifa = NULL;
		void* tmpAddrPtr = NULL;
		if (getifaddrs(&ifAddrStruct) != -1)
		{
			for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
			{
				if (ifa ->ifa_addr->sa_family == AF_INET)
				{ // check it is IP4
					// is a valid IP4 Address
					tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
					char addressBuffer[INET_ADDRSTRLEN] = {'\0'};
					inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
					//printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
				}
				else if (ifa->ifa_addr->sa_family == AF_INET6)
				{ // check it is IP6
					// is a valid IP6 Address
					tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
					char addressBuffer[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
					//printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
				}
			}
			freeifaddrs(ifAddrStruct);
		}
		*/
#endif
		return result;
	}

}
#endif
