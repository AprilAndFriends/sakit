/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WIN32) && defined(_WINRT)
#define _NO_WIN_H
#include <hltypes/hplatform.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "sakit.h"
#include "PlatformSocket.h"

using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;

namespace sakit
{
	extern int bufferSize;

	// TODOsock - implement and test this properly
	void PlatformSocket::platformInit()
	{
	}

	void PlatformSocket::platformDestroy()
	{
	}

	PlatformSocket::PlatformSocket() : connected(false), connectionLess(false)
	{
		this->sSock = nullptr;
		this->dSock = nullptr;
		this->sServer = nullptr;
		this->bufferSize = sakit::bufferSize;
		this->receiveBuffer = new char[this->bufferSize];
		memset(this->receiveBuffer, 0, this->bufferSize);
	}

	bool PlatformSocket::_awaitAsync(Result& result, hmutex& mutex)
	{
		// TODOsock - add timeouts from settings?
		int i = 0;
		while (true)
		{
			mutex.lock();
			if (result == FINISHED)
			{
				mutex.unlock();
				break;
			}
			if (i >= 10000)
			{
				result = FAILED;
				mutex.unlock();
				PlatformSocket::_printLastError("Async Timeout!");
				break;
			}
			hthread::sleep(1.0f);
			i++;
		}
		return (result != FAILED);
	}

	Windows::Networking::HostName^ PlatformSocket::_makeHostName(Host host)
	{
		Windows::Networking::HostName^ hostName = nullptr;
		try
		{
			hostName = ref new HostName(_HL_HSTR_TO_PSTR(host.toString()));
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return nullptr;
		}
		return hostName;
	}

	bool PlatformSocket::createSocket(Host host, unsigned short port)
	{
		this->connected = true;
		// create host info
		HostName^ hostName = PlatformSocket::_makeHostName(host);
		if (hostName == nullptr)
		{
			this->disconnect();
			return false;
		}
		// create socket
		if (this->_server)
		{
			this->sServer = ref new StreamSocketListener();
		}
		else if (!this->connectionLess)
		{
			this->sSock = ref new StreamSocket();
			this->sSock->Control->KeepAlive = false; // TODOsock - check if this is ok
			this->sSock->Control->NoDelay = true;
		}
		else
		{
			this->dSock = ref new DatagramSocket();
		}
		return true;
	}

	bool PlatformSocket::connect(Host host, unsigned short port)
	{
		this->_server = false;
		if (!this->createSocket(host, port))
		{
			return false;
		}
		// create host info
		HostName^ hostName = PlatformSocket::_makeHostName(host);
		if (hostName == nullptr)
		{
			this->disconnect();
			return false;
		}
		// open socket
		this->_asyncConnected = false;
		this->_asyncConnectionResult = RUNNING;
		IAsyncAction^ action = this->sSock->ConnectAsync(hostName, _HL_HSTR_TO_PSTR(hstr(port)), SocketProtectionLevel::PlainSocket);
		action->Completed = ref new AsyncActionCompletedHandler([this](IAsyncAction^ action, AsyncStatus status)
		{
			this->_mutexConnection.lock();
			if (this->_asyncConnectionResult == RUNNING)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncConnected = true;
				}
				this->_asyncConnectionResult = FINISHED;
			}
			this->_mutexConnection.unlock();
		});
		if (!PlatformSocket::_awaitAsync(this->_asyncConnectionResult, this->_mutexConnection))
		{
			action->Cancel();
			this->disconnect();
			return false;
		}
		if (!this->_asyncConnected)
		{
			this->disconnect();
		}
		return this->_asyncConnected;
	}

	bool PlatformSocket::bind(Host host, unsigned short port)
	{
		this->_server = true;
		if (!this->createSocket(host, port))
		{
			return false;
		}
		// create host info
		HostName^ hostName = PlatformSocket::_makeHostName(host);
		if (hostName == nullptr)
		{
			this->disconnect();
			return false;
		}
		this->_asyncBound = false;
		this->_asyncBindingResult = RUNNING;
		IAsyncAction^ action = this->sServer->BindEndpointAsync(hostName, _HL_HSTR_TO_PSTR(hstr(port)));
		action->Completed = ref new AsyncActionCompletedHandler([this](IAsyncAction^ action, AsyncStatus status)
		{
			this->_mutexBinder.lock();
			if (this->_asyncBindingResult == RUNNING)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncBound = true;
				}
				this->_asyncBindingResult = FINISHED;
			}
			this->_mutexBinder.unlock();
		});
		if (!PlatformSocket::_awaitAsync(this->_asyncBindingResult, this->_mutexBinder))
		{
			action->Cancel();
			this->disconnect();
			return false;
		}
		if (!this->_asyncBound)
		{
			this->disconnect();
		}
		return this->_asyncBound;
	}

	bool PlatformSocket::joinMulticastGroup(Host host, unsigned short port, Host groupAddress)
	{
		return false;
		/*
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
		sockaddr_in local;
		local.sin_family = ai_family;
		local.sin_port = htons(port);
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		if (!this->_checkResult(::bind(this->sock, (sockaddr*)&local, sizeof(sockaddr_in)), "bind()"))
		{
			return false;
		}
		ip_mreq group;
		group.imr_interface.s_addr = inet_addr(host.toString().c_str());
		group.imr_multiaddr.s_addr = inet_addr(groupAddress.toString().c_str());
		if (!this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(ip_mreq)), "setsockopt()"))
		{
			return false;
		}
		this->multicastGroupAddress.sin_family = ai_family;
		this->multicastGroupAddress.sin_addr.s_addr = inet_addr(groupAddress.toString().c_str());
		this->multicastGroupAddress.sin_port = htons(port);
		return this->setMulticastTtl(32);
		*/
	}

	bool PlatformSocket::setMulticastInterface(Host address)
	{
		return false;
		/*
		in_addr local;
		local.s_addr = inet_addr(address.toString().c_str());
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&local, sizeof(in_addr)), "setsockopt()");
		*/
	}

	bool PlatformSocket::setMulticastTtl(int value)
	{
		return false;
		/*
		int ttl = 20;
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(int)), "setsockopt()");
		*/
	}

	bool PlatformSocket::setMulticastLoopback(bool value)
	{
		return false;
		/*
		int loopBack = (value ? 1 : 0);
		return this->_checkResult(setsockopt(this->sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopBack, sizeof(int)), "setsockopt()");
		*/
	}

	bool PlatformSocket::disconnect()
	{
		if (this->sSock != nullptr)
		{
			delete this->sSock; // deleting the socket is the documented way in WinRT to close the socket in C++
			this->sSock = nullptr;
		}
		if (this->dSock != nullptr)
		{
			delete this->dSock; // deleting the socket is the documented way in WinRT to close the socket in C++
			this->dSock = nullptr;
		}
		if (this->sServer != nullptr)
		{
			delete this->sServer; // deleting the socket is the documented way in WinRT to close the socket in C++
			this->sServer = nullptr;
		}
		bool previouslyConnected = this->connected;
		this->connected = false;
		return previouslyConnected;
	}

	bool PlatformSocket::send(hstream* stream, int& count, int& sent)
	{
		this->_asyncSent = false;
		this->_asyncSendingResult = RUNNING;
		this->_asyncSentSize = 0;
		unsigned char* data = (unsigned char*)&(*stream)[stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		DataWriter^ writer = ref new DataWriter();
		writer->WriteBytes(ref new Platform::Array<unsigned char>(data, size));
		IAsyncOperationWithProgress<unsigned int, unsigned int>^ operation = nullptr;
		if (this->sSock != nullptr)
		{
			operation = this->sSock->OutputStream->WriteAsync(writer->DetachBuffer());
		}
		if (this->dSock != nullptr)
		{
			operation = this->dSock->OutputStream->WriteAsync(writer->DetachBuffer());
		}
		operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<unsigned int, unsigned int>(
			[this](IAsyncOperationWithProgress<unsigned int, unsigned int>^ operation, AsyncStatus status)
		{
			this->_mutexSender.lock();
			if (this->_asyncSendingResult == RUNNING)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncSent = true;
					this->_asyncSentSize = operation->GetResults();
				}
				this->_asyncSendingResult = FINISHED;
			}
			this->_mutexSender.unlock();
		});
		if (!PlatformSocket::_awaitAsync(this->_asyncSendingResult, this->_mutexSender))
		{
			operation->Cancel();
			this->disconnect();
			return false;
		}
		if (this->_asyncSentSize >= 0)
		{
			stream->seek(this->_asyncSentSize);
			sent += this->_asyncSentSize;
			count -= this->_asyncSentSize;
			return true;
		}
		return this->_asyncSent;
		// TODOsock - are all scenarios covered?
		/*
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
		else if (this->address != NULL)
		{
			result = ::sendto(this->sock, data, size, 0, (sockaddr*)this->address, sizeof(*this->address));
		}
		else
		{
			result = ::sendto(this->sock, data, size, 0, (sockaddr*)&this->multicastGroupAddress, sizeof(this->multicastGroupAddress));
		}
		if (result >= 0)
		{
			stream->seek(result);
			sent += result;
			count -= result;
			return true;
		}
		return false;
		*/
	}

	bool PlatformSocket::receive(hstream* stream, hmutex& mutex, int& count)
	{
		this->_asyncReceived = false;
		this->_asyncReceivingResult = RUNNING;
		this->_asyncReceivedSize = 0;
		int read = (count > 0 ? this->bufferSize : hmin(this->bufferSize, count));
		Buffer^ _buffer = ref new Buffer(read);
		IAsyncOperationWithProgress<IBuffer^, unsigned int>^ operation = nullptr;
		if (this->sSock != nullptr)
		{
			operation = this->sSock->InputStream->ReadAsync(_buffer, read, InputStreamOptions::None);
		}
		// TODOsock - UDP uses the same way?
		/*
		if (this->dSock != nullptr)
		{
			operation = this->dSock->>InputStream->ReadAsync(_buffer, read, InputStreamOptions::None);
		}
		*/
		operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, unsigned int>(
			[this](IAsyncOperationWithProgress<IBuffer^, unsigned int>^ operation, AsyncStatus status)
		{
			this->_mutexReceiver.lock();
			if (this->_asyncReceivingResult == RUNNING)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncReceived = true;
					this->_asyncReceivedSize = operation->GetResults()->Length;
				}
				this->_asyncReceivingResult = FINISHED;
			}
			this->_mutexReceiver.unlock();
		});
		if (!PlatformSocket::_awaitAsync(this->_asyncReceivingResult, this->_mutexReceiver))
		{
			operation->Cancel();
			this->disconnect();
			return false;
		}
		if (this->_asyncReceivedSize > 0)
		{
			Platform::Array<unsigned char>^ _data = ref new Platform::Array<unsigned char>(_buffer->Length);
			try
			{
				DataReader::FromBuffer(_buffer)->ReadBytes(_data);
			}
			catch (Platform::OutOfBoundsException^ e)
			{
				return false;
			}
			mutex.lock();
			stream->write_raw(_data->Data, _data->Length);
			this->_asyncReceivedSize = (int)_data->Length;
			mutex.unlock();
			if (count > 0) // if don't read everything
			{
				count -= this->_asyncReceivedSize;
			}
		}
		return true;
		// TODOsock - are all scenarios covered?
		/*
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
		if (count > 0) // if don't read everything
		{
			read = hmin(read, count);
		}
		read = recv(this->sock, this->receiveBuffer, read, 0);
		if (!this->_checkResult(read, "recv()", false))
		{
			return false;
		}
		mutex.lock();
		stream->write_raw(this->receiveBuffer, read);
		mutex.unlock();
		if (count > 0) // if don't read everything
		{
			count -= read;
		}
		return true;
		*/
	}

	bool PlatformSocket::listen()
	{
		return false;
		/*
		return this->_checkResult(::listen(this->sock, SOMAXCONN), "listen()", false);
		*/
	}

	bool PlatformSocket::accept(Socket* socket)
	{
		return false;
		/*
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
		char hostString[CHAR_BUFFER] = {'\0'};
		char portString[CHAR_BUFFER] = {'\0'};
		getnameinfo((sockaddr*)other->address, size, hostString, CHAR_BUFFER, portString, CHAR_BUFFER, NI_NUMERICHOST);
		((Base*)socket)->_activateConnection(Host(hostString), (unsigned short)(int)hstr(portString));
		other->connected = true;
		return true;
		*/
	}

	bool PlatformSocket::receiveFrom(hstream* stream, Socket* socket)
	{
		return false;
		/*
		PlatformSocket* other = socket->socket;
		socklen_t size = (socklen_t)sizeof(sockaddr_storage);
		other->address = (sockaddr_storage*)malloc(size);
		this->_setNonBlocking(true);
		int received = recvfrom(this->sock, this->receiveBuffer, this->bufferSize, 0, (sockaddr*)other->address, &size);
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
			((Base*)socket)->_activateConnection(Host(hostString), (unsigned short)(int)hstr(portString));
		}
		return true;
		*/
	}

	bool PlatformSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count)
	{
		return false;
		/*
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
			PlatformSocket::_printLastError("socket()");
			closesocket(sock);
			return false;
		}
		int broadcast = 1;
		result = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast));
		if (result == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError("setsockopt()");
			closesocket(sock);
			return false;
		}
		int reuseaddress = 1;
		result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddress, sizeof(reuseaddress));
		if (result == SOCKET_ERROR)
		{
			PlatformSocket::_printLastError("setsockopt()");
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
			address.sin_addr.s_addr = inet_addr((*it).getBroadcastIp().toString().c_str());
			result = sendto(sock, data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));
			if (result == SOCKET_ERROR)
			{
				PlatformSocket::_printLastError("sendto()");
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
		*/
	}

	hstr PlatformSocket::resolveHost(chstr domain)
	{
		return PlatformSocket::_resolve(domain, true);
	}

	hstr PlatformSocket::resolveIp(chstr ip)
	{
		// wow, Microsoft, just wow
		hlog::warn(sakit::logTag, "WinRT does not support resolving an IP address to a host name. Attempting anyway, but don't count on it.");
		return PlatformSocket::_resolve(ip, false);
	}

	hstr PlatformSocket::_resolve(chstr host, bool wantIp)
	{
		Windows::Networking::HostName^ hostName = nullptr;
		// create host info
		try
		{
			hostName = ref new HostName(_HL_HSTR_TO_PSTR(host));
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return "";
		}
		IAsyncOperation<Collections::IVectorView<EndpointPair^>^>^ operation = DatagramSocket::GetEndpointPairsAsync(hostName, "0");
		Result _asyncFinished = RUNNING;
		hstr result;
		hmutex mutex;
		operation->Completed = ref new AsyncOperationCompletedHandler<Collections::IVectorView<EndpointPair^>^>(
			[wantIp, &mutex, &_asyncFinished, &result](IAsyncOperation<Collections::IVectorView<EndpointPair^>^>^ operation, AsyncStatus status)
		{
			mutex.lock();
			if (_asyncFinished == RUNNING)
			{
				if (status == AsyncStatus::Completed)
				{
					Collections::IVectorView<EndpointPair^>^ endpointPairs = operation->GetResults();
					if (endpointPairs != nullptr && endpointPairs->Size > 0)
					{
						HostNameType type = (wantIp ? HostNameType::Ipv4 : HostNameType::DomainName);
						for (Collections::IIterator<EndpointPair^>^ it = endpointPairs->First(); it->HasCurrent; it->MoveNext())
						{
							if (it->Current->RemoteHostName != nullptr && it->Current->RemoteHostName->Type == type)
							{
								result = _HL_PSTR_TO_HSTR(it->Current->RemoteHostName->DisplayName);
								break;
							}
						}
					}
				}
				_asyncFinished = FINISHED;
			}
			mutex.unlock();
		});
		if (!PlatformSocket::_awaitAsync(_asyncFinished, mutex))
		{
			operation->Cancel();
			mutex.unlock();
			return "";
		}
		return result;
	}

	harray<NetworkAdapter> PlatformSocket::getNetworkAdapters()
	{
		harray<NetworkAdapter> result;
		/*
		Collections::IVectorView<ConnectionProfile^>^ profiles = NetworkInformation::GetConnectionProfiles();
		for (Collections::IIterator<ConnectionProfile^>^ it = profiles->First(); it->HasCurrent; it->MoveNext())
		{
			Collections::IVectorView<Platform::String^>^ names = it->Current->GetNetworkNames();
			for (Collections::IIterator<Platform::String^>^ it2 = names->First(); it2->HasCurrent; it2->MoveNext())
			{
				hlog::debug("OK", _HL_PSTR_TO_HSTR(it2->Current));
			}
			hlog::debug("--", "");
			hlog::debug("OK", _HL_PSTR_TO_HSTR(it->Current->ProfileName));
			hlog::debug("OK", _HL_PSTR_TO_HSTR(it->Current->NetworkAdapter->NetworkAdapterId.ToString()));
			hlog::debug("OK", _HL_PSTR_TO_HSTR(it->Current->NetworkAdapter->NetworkItem->NetworkId.ToString()));
			hlog::debug("OK", _HL_PSTR_TO_HSTR(it->Current->NetworkAdapter->ToString()));
		}
		hlog::debug("--", "");
		*/
		Collections::IVectorView<HostName^>^ hostNames = NetworkInformation::GetHostNames();
		for (Collections::IIterator<HostName^>^ it = hostNames->First(); it->HasCurrent; it->MoveNext())
		{
			//hlog::debug("HN", _HL_PSTR_TO_HSTR(it->Current->DisplayName));
			result += NetworkAdapter(0, 0, "", "", "", Host(_HL_PSTR_TO_HSTR(it->Current->DisplayName)), Host(), Host());
		}
		/*
		hlog::debug("--", "");
		Collections::IVectorView<LanIdentifier^>^ lanIdentifiers = NetworkInformation::GetLanIdentifiers();
		for (Collections::IIterator<LanIdentifier^>^ it = lanIdentifiers->First(); it->HasCurrent; it->MoveNext())
		{
			hlog::debug("LL", _HL_PSTR_TO_HSTR(it->Current->NetworkAdapterId.ToString()));
			hlog::debug("LL", hstr(it->Current->PortId->Type));
			hlog::debug("LL", _HL_PSTR_TO_HSTR(it->Current->PortId->Value->ToString()));
			hlog::debug("LL", hstr(it->Current->InfrastructureId->Type));
			hlog::debug("LL", _HL_PSTR_TO_HSTR(it->Current->InfrastructureId->Value->ToString()));
		}
		hlog::debug("--", "");
		*/
		return result;
	}

}
#endif
