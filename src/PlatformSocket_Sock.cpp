/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if !defined(_WIN32) || !defined(_WINRT)
#include <stdint.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <memory.h>
#define IN_ADDRT_T_TYPECAST (in_addr_t)
#else
#define IN_ADDRT_T_TYPECAST
#endif

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
#ifndef _ANDROID
#include <ifaddrs.h>
#else
#include "ifaddrs_android.h"
#endif
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
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Server.h"
#include "Socket.h"

#ifdef _IOS
	#define FAMILY_INET AF_INET6
	#define FAMILY_CONNECT_INET AF_INET
	#define USE_FALLBACK
#elif !defined(_ANDROID)
	#define FAMILY_INET AF_INET
	#define FAMILY_CONNECT_INET AF_INET
#else
	#define FAMILY_INET PF_INET
	#define FAMILY_CONNECT_INET PF_INET
#endif

namespace sakit
{
	extern int bufferSize;
	// even though by standard definition these functions should be thread-safe, practice has shown otherwise
	static hmutex mutexGetaddrinfo;
	static hmutex mutexFreeaddrinfo;
	static hmutex mutexGetnameinfo;
	static hmutex mutexGetsockname;

	// utility functions
#ifdef _WIN32
	#define __gai_strerror(str) hstr::fromUnicode(gai_strerrorW(str))

	// inet_pton() is not supported on WinXP so we provide a native Win32 implementation
	static int __inet_pton(int family, const char* src, void* dest)
	{
		struct sockaddr_storage address;
		int size = sizeof(sockaddr_storage);
		hstr ip = src;
		if (WSAStringToAddressW((wchar_t*)ip.wStr().c_str(), family, NULL, (sockaddr*)&address, &size) == 0)
		{
			switch (family)
			{
			case AF_INET:
				*((in_addr*)dest) = ((sockaddr_in*)&address)->sin_addr;
				return 1;
			case AF_INET6:
				*((in6_addr*)dest) = ((sockaddr_in6*)&address)->sin6_addr;
				return 1;
			}
		}
		return 0;
	}
#else
	#define __gai_strerror(x) hstr(gai_strerror(x))
	#define __inet_pton inet_pton
#endif

	// wrappers for thread-unsafe functions
	static hmutex mutexInetNtoa;
	static hmutex mutexInetAddr;
	static hmutex mutexHtons;
	static hmutex mutexNtohs;

	static char* __inet_ntoa(struct in_addr in)
	{
		hmutex::ScopeLock lock(&mutexInetNtoa);
		return inet_ntoa(in);
	}

	static unsigned long __inet_addr(const char* cp)
	{
		hmutex::ScopeLock lock(&mutexInetAddr);
		return inet_addr(cp);
	}

	static unsigned short __htons(unsigned hostshort)
	{
		hmutex::ScopeLock lock(&mutexHtons);
		return htons(hostshort);
	}

	static unsigned short __ntohs(unsigned netshort)
	{
		hmutex::ScopeLock lock(&mutexNtohs);
		return ntohs(netshort);
	}

	// normal methods

	void PlatformSocket::platformInit()
	{
#if defined(_WIN32) && !defined(_WINRT)
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0)
		{
			hlog::error(logTag, "Error: " + hstr(result));
		}
#endif
	}

	void PlatformSocket::platformDestroy()
	{
#if defined(_WIN32) && !defined(_WINRT)
		int result = WSACleanup();
		if (result != 0)
		{
			hlog::error(logTag, "Error: " + hstr(result));
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
		return this->_checkResult(ioctlsocket(this->sock, FIONBIO, (unsigned long*)&setValue), "ioctlsocket()");
	}

	bool PlatformSocket::tryCreateSocket()
	{
		if (this->sock == (unsigned int)-1)
		{
			this->connected = true;
			this->sock = socket(this->socketInfo->ai_family, this->socketInfo->ai_socktype, this->socketInfo->ai_protocol);
			return this->_checkResult(this->sock, "socket()");
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
		hmutex::ScopeLock lock;
		if (*info != NULL)
		{
			lock.acquire(&mutexFreeaddrinfo);
			freeaddrinfo(*info);
			lock.release();
			*info = NULL;
		}
		this->socketInfo->ai_family = FAMILY_INET;
		this->socketInfo->ai_socktype = (!this->connectionLess ? SOCK_STREAM : SOCK_DGRAM);
		this->socketInfo->ai_protocol = IPPROTO_IP;
		this->socketInfo->ai_flags = 0;
		lock.acquire(&mutexGetaddrinfo);
		int result = getaddrinfo(host.toString().cStr(), hstr(port).cStr(), this->socketInfo, info);
#ifdef USE_FALLBACK
		if (result != 0)
		{
			this->socketInfo->ai_family = AF_INET;
			result = getaddrinfo(host.toString().cStr(), hstr(port).cStr(), this->socketInfo, info);
		}
#endif
		if (result != 0)
		{
			hlog::error(logTag, "getaddrinfo() " + __gai_strerror(result));
			lock.release();
			this->disconnect();
			return false;
		}
		lock.release();
		this->socketInfo->ai_family = (*info)->ai_family;
		this->socketInfo->ai_socktype = (*info)->ai_socktype;
		this->socketInfo->ai_protocol = (*info)->ai_protocol;
		return true;
	}

	bool PlatformSocket::connect(Host remoteHost, unsigned short remotePort, Host& localHost, unsigned short& localPort, float timeout, float retryFrequency)
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
		this->_setNonBlocking(true);
		int result = ::connect(this->sock, this->remoteInfo->ai_addr, this->remoteInfo->ai_addrlen);
		this->_setNonBlocking(false);
		if (result != 0) // uses non-blocking
		{
			if (PlatformSocket::_printLastError("connect()")) // failed and actual error
			{
				this->disconnect();
				return false;
			}
			// non-blocking mode, use select to check when it finally worked
			double sec = (double)timeout;
			timeval interval = {0, 1};
			interval.tv_sec = (long)sec;
			if (sec != (int)sec)
			{
#ifdef __APPLE__
				interval.tv_usec = (int)((sec - (int)sec) * 1000000);
#else
				interval.tv_usec = (long)((sec - (int)sec) * 1000000);
#endif
			}
			fd_set writeSet;
			FD_ZERO(&writeSet);
			FD_SET(this->sock, &writeSet);
			result = select(this->sock + 1, NULL, &writeSet, NULL, &interval);
			if (result == 0)
			{
				hlog::error(logTag, "Unable to connect, timed out.");
				this->disconnect();
				return false;
			}
			if (!this->_checkResult(result, "select()"))
			{
				return false;
			}
			int error;
			socklen_t size = sizeof(error);
			result = getsockopt(this->sock, SOL_SOCKET, SO_ERROR, (char*)&error, &size);
			if (!this->_checkResult(result, "getsockopt()"))
			{
				return false;
			}
			if (PlatformSocket::_printLastError("", error))
			{
				this->disconnect();
				return false;
			}
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
		address.sin_addr.s_addr = IN_ADDRT_T_TYPECAST __inet_addr(host.toString().cStr());
		address.sin_port = __htons(port);
		hmutex::ScopeLock lock(&mutexGetsockname);
		getsockname(this->sock, (sockaddr*)&address, &addressSize);
		lock.release();
		host = Host(__inet_ntoa(address.sin_addr));
		port = __ntohs(address.sin_port);
	}

	bool PlatformSocket::joinMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		ip_mreq group;
		group.imr_interface.s_addr = IN_ADDRT_T_TYPECAST __inet_addr(interfaceHost.toString().cStr());
		group.imr_multiaddr.s_addr = IN_ADDRT_T_TYPECAST __inet_addr(groupAddress.toString().cStr());
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(ip_mreq)), "setsockopt()");
	}

	bool PlatformSocket::leaveMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		ip_mreq group;
		group.imr_interface.s_addr = IN_ADDRT_T_TYPECAST __inet_addr(interfaceHost.toString().cStr());
		group.imr_multiaddr.s_addr = IN_ADDRT_T_TYPECAST __inet_addr(groupAddress.toString().cStr());
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
		local.s_addr = IN_ADDRT_T_TYPECAST __inet_addr(interfaceHost.toString().cStr());
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
		hmutex::ScopeLock lock(&mutexFreeaddrinfo);
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
		lock.release();
		if (this->address != NULL)
		{
			free(this->address);
			this->address = NULL;
		}
		if (this->sock != (unsigned int)-1)
		{
			closesocket(this->sock);
			this->sock = (unsigned int)-1;
		}
		bool previouslyConnected = this->connected;
		this->connected = false;
		return previouslyConnected;
	}

	bool PlatformSocket::send(hstream* stream, int& count, int& sent)
	{
		const char* data = (const char*)&(*stream)[(int)stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		int result = 0;
		if (!this->connectionLess)
		{
			result = (int)::send(this->sock, data, size, 0);
		}
		else if (this->remoteInfo != NULL)
		{
			result = (int)::sendto(this->sock, data, size, 0, this->remoteInfo->ai_addr, this->remoteInfo->ai_addrlen);
		}
		else if (this->address != NULL)
		{
			result = (int)::sendto(this->sock, data, size, 0, (sockaddr*)this->address, sizeof(*this->address));
		}
		else
		{
			hlog::warn(logTag, "Trying to send without a remote host!");
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

	bool PlatformSocket::receive(hstream* stream, int& maxCount, hmutex* mutex)
	{
		unsigned long receivedCount = 0;
		if (!this->_checkReceivedCount(&receivedCount))
		{
			return false;
		}
		if (receivedCount == 0)
		{
			return true;
		}
		int readCount = hmin((int)receivedCount, this->bufferSize);
		if (maxCount > 0) // if don't read everything
		{
			readCount = hmin(readCount, maxCount);
		}
		readCount = (int)recv(this->sock, this->receiveBuffer, readCount, 0);
		if (!this->_checkResult(readCount, "recv()", false))
		{
			return false;
		}
		hmutex::ScopeLock lock(mutex);
		stream->writeRaw(this->receiveBuffer, readCount);
		lock.release();
		if (maxCount > 0) // if not trying to read everything at once
		{
			maxCount -= readCount;
		}
		return true;
	}

	bool PlatformSocket::receiveFrom(hstream* stream, Host& remoteHost, unsigned short& remotePort)
	{
		unsigned long receivedCount = 0;
		if (!this->_checkReceivedCount(&receivedCount))
		{
			return false;
		}
		if (receivedCount == 0)
		{
			return true;
		}
		int read = hmin((int)receivedCount, this->bufferSize);
		sockaddr_storage address;
		socklen_t size = (socklen_t)sizeof(sockaddr_storage);
		this->_setNonBlocking(true);
		read = (int)recvfrom(this->sock, this->receiveBuffer, read, 0, (sockaddr*)&address, &size);
		if (!this->_checkResult(read, "recvfrom()"))
		{
			this->_setNonBlocking(false);
			return false;
		}
		this->_setNonBlocking(false);
		if (read > 0)
		{
			stream->writeRaw(this->receiveBuffer, read);
			// get the IP and port of the connected client
			char hostString[NI_MAXHOST] = {'\0'};
			char portString[NI_MAXSERV] = {'\0'};
			hmutex::ScopeLock lock(&mutexGetnameinfo);
			getnameinfo((sockaddr*)&address, size, hostString, NI_MAXHOST, portString, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
			lock.release();
			remoteHost = Host(hostString);
			remotePort = (unsigned short)(int)hstr(portString);
		}
		return true;
	}

	bool PlatformSocket::_checkReceivedCount(unsigned long* receivedCount)
	{
#ifndef _WIN32 // Unix requires a select() call before using ioctl/ioctlsocket
		timeval interval = {0, 1};
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(this->sock, &readSet);
		int result = select(this->sock + 1, &readSet, NULL, NULL, &interval);
		if (!this->_checkResult(result, "select()"))
		{
			return false;
		}
		if (result == 0)
		{
			return true;
		}
#endif
		// control socket IO
		return this->_checkResult(ioctlsocket(this->sock, FIONREAD, (unsigned long*)receivedCount), "ioctlsocket()", false);
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
		hmutex::ScopeLock lock(&mutexGetnameinfo);
		getnameinfo((sockaddr*)other->address, size, hostString, NI_MAXHOST, portString, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
		lock.release();
		Host localHost;
		unsigned short localPort = 0;
		this->_getLocalHostPort(localHost, localPort);
		((SocketBase*)socket)->_activateConnection(Host(hostString), (unsigned short)(int)hstr(portString), localHost, localPort);
		other->connected = true;
		return true;
	}

	bool PlatformSocket::_checkResult(int result, chstr functionName, bool disconnectOnError)
	{
		if (result < 0)
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
		const char* data = (const char*)&(*stream)[(int)stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		int broadcast = 1;
		if (!this->_checkResult(setsockopt(this->sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)), "setsockopt", false))
		{
			return false;
		}
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
		address.sin_family = FAMILY_CONNECT_INET;
		address.sin_port = __htons(port);
		int result = 0;
		int maxResult = 0;
		socklen_t addrSize = sizeof(sockaddr_in);
		Host broadcastIp;
		HL_LAMBDA_CLASS(_adapterHosts, Host, ((NetworkAdapter const& adapter) { return adapter.getBroadcastIp(); }));
		harray<Host> ips = adapters.mapped(&_adapterHosts::lambda);
		ips.removeDuplicates(); // to avoid broadcasting on the same IP twice, just to be sure
		foreach (Host, it, ips)
		{
			address.sin_addr.s_addr = IN_ADDRT_T_TYPECAST __inet_addr((*it).toString().cStr());
			result = (int)sendto(this->sock, data, size, 0, (sockaddr*)&address, addrSize);
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
		hints.ai_family = FAMILY_CONNECT_INET;
		hmutex::ScopeLock lock(&mutexGetaddrinfo);
		int result = getaddrinfo(domain.toString().cStr(), NULL, &hints, &info);
		if (result != 0)
		{
			hlog::error(logTag, __gai_strerror(result));
			return Host();
		}
		lock.release();
		in_addr address;
		address.s_addr = ((sockaddr_in*)(info->ai_addr))->sin_addr.s_addr;
		lock.acquire(&mutexFreeaddrinfo);
		freeaddrinfo(info);
		lock.release();
		return Host(__inet_ntoa(address));
	}

	Host PlatformSocket::resolveIp(Host ip)
	{
		sockaddr_in address;
		address.sin_family = FAMILY_CONNECT_INET;
		__inet_pton(address.sin_family, ip.toString().cStr(), &address.sin_addr);
		char hostName[NI_MAXHOST] = {'\0'};
		hmutex::ScopeLock lock(&mutexGetnameinfo);
		int result = getnameinfo((sockaddr*)&address, sizeof(address), hostName, sizeof(hostName), NULL, 0, NI_NUMERICHOST);
		if (result != 0)
		{
			hlog::error(logTag, __gai_strerror(result));
			return Host();
		}
		return Host(hostName);
	}

	unsigned short PlatformSocket::resolveServiceName(chstr serviceName)
	{
		addrinfo hints;
		addrinfo* info;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = FAMILY_CONNECT_INET;
		hmutex::ScopeLock lock(&mutexGetaddrinfo);
		int result = getaddrinfo(NULL, serviceName.cStr(), &hints, &info);
		if (result != 0)
		{
			hlog::error(logTag, __gai_strerror(result));
			return 0;
		}
		lock.release();
		unsigned short port = ((sockaddr_in*)(info->ai_addr))->sin_port;
		lock.acquire(&mutexFreeaddrinfo);
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
			hlog::error(logTag, "Not enough memory!");
			return result;
		}
		if (GetAdaptersInfo(info, &size) == ERROR_BUFFER_OVERFLOW) // gets the size required if necessary
		{
			free(info);
			info = (IP_ADAPTER_INFO*)malloc(size);
			if (info == NULL)
			{
				hlog::error(logTag, "Not enough memory!");
				return result;
			}
		}
		if ((GetAdaptersInfo(info, &size)) != NO_ERROR)
		{
			hlog::error(logTag, "Not enough memory!");
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
			gateway = Host("0.0.0.0");
			result += NetworkAdapter(comboIndex, index, name, description, type, address, mask, gateway);
			pAdapter = pAdapter->Next;
		}
#else
		struct ifaddrs* ifaddr = NULL;
		struct ifaddrs* ifa = NULL;
		int family;
		Host host;
		Host mask;
		Host gateway;
		hstr name;
		hstr description;
		hstr type;
		if (getifaddrs(&ifaddr) != -1)
		{
			for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr != NULL)
				{
					family = ifa->ifa_addr->sa_family;
					if (family == AF_INET)// || family == AF_INET6) // sakit only supports IPV4 for now
					{
						host = __inet_ntoa(((sockaddr_in*)ifa->ifa_addr)->sin_addr);
						mask = __inet_ntoa(((sockaddr_in*)ifa->ifa_netmask)->sin_addr);
						if (ifa->ifa_dstaddr != NULL)
						{
							gateway = __inet_ntoa(((sockaddr_in*)ifa->ifa_dstaddr)->sin_addr);
						}
						else // when it's localhost, gateway can be NULL
						{
							gateway = Host::Any;
						}
						name = ifa->ifa_name;
						description = name + " network adapter";
						result += NetworkAdapter(0, 0, name, description, type, host, mask, "");
					}
				}
			}
			freeifaddrs(ifaddr);
		}
#endif
		return result;
	}

}
#endif
