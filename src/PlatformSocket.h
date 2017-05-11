/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a platform dependent implementation for socket functionality.

#ifndef SAKIT_PLATFORM_SOCKET_H
#define SAKIT_PLATFORM_SOCKET_H

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "Host.h"
#include "NetworkAdapter.h"
#include "State.h"

#ifdef __APPLE__
#include <netinet/in.h>
#endif
#ifdef _WINRT
using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
#endif

namespace sakit
{
	class Socket;

	class PlatformSocket
	{
	public:
		PlatformSocket();
		~PlatformSocket();

		HL_DEFINE_IS(connected, Connected);
		HL_DEFINE_ISSET(connectionLess, ConnectionLess);
		HL_DEFINE_ISSET(serverMode, ServerMode); // actually used only in WinRT

		bool tryCreateSocket();
		bool setRemoteAddress(Host remoteHost, unsigned short remotePort);
		bool setLocalAddress(Host localHost, unsigned short localPort);
		bool connect(Host remoteHost, unsigned short remotePort, Host& localHost, unsigned short& localPort, float timeout, float retryFrequency);
		/// @note Since binding can be done on "any IP" and "any port", the set values are returned.
		bool bind(Host localHost, unsigned short& localPort);
		bool disconnect();
		bool send(hstream* stream, int& sent, int& count);
		bool receive(hstream* stream, int& maxCount, hmutex* mutex = NULL);
		bool receiveFrom(hstream* stream, Host& remoteHost, unsigned short& remotePort);
		bool listen();
		bool accept(Socket* socket);

		bool broadcast(harray<NetworkAdapter> adapters, unsigned short remotePort, hstream* stream, int count);
		bool joinMulticastGroup(Host interfaceHost, Host groupAddress);
		bool leaveMulticastGroup(Host interfaceHost, Host groupAddress);

		bool setNagleAlgorithmActive(bool value);
		bool setMulticastInterface(Host interfaceHost);
		bool setMulticastTtl(int value);
		bool setMulticastLoopback(bool value);

		static Host resolveHost(Host domain);
		static Host resolveIp(Host ip);
		static unsigned short resolveServiceName(chstr serviceName);
		static harray<NetworkAdapter> getNetworkAdapters();
		
		static void platformInit();
		static void platformDestroy();

	protected:
		bool connected;
		bool connectionLess;
		char* receiveBuffer;
		int bufferSize;
		bool serverMode;

#if !defined(_WIN32) || !defined(_WINRT)
		unsigned int sock;
		struct addrinfo* socketInfo;
		struct addrinfo* localInfo;
		struct addrinfo* remoteInfo;
		struct sockaddr_storage* address;

		bool _setAddress(Host& host, unsigned short& port, addrinfo** info);
		bool _checkReceivedCount(unsigned long* receivedCount);
		bool _checkResult(int result, chstr functionName, bool disconnectOnError = true);
		void _getLocalHostPort(Host& host, unsigned short& port);
#else
		// there is no other way to make this work
		[Windows::Foundation::Metadata::WebHostHidden]
		ref class ConnectionAccepter sealed
		{
		public:
			friend class PlatformSocket;

			ConnectionAccepter() { }

			virtual void onConnectedStream(StreamSocketListener^ listener, StreamSocketListenerConnectionReceivedEventArgs^ args);

		private:
			PlatformSocket* socket;

			~ConnectionAccepter() { }

		};

		// there is no other way to make this work
		[Windows::Foundation::Metadata::WebHostHidden]
		ref class UdpReceiver sealed
		{
		public:
			friend class PlatformSocket;

			UdpReceiver() { }

			virtual void onReceivedDatagram(DatagramSocket^ socket, DatagramSocketMessageReceivedEventArgs^ args);
			
		private:
			PlatformSocket* socket;
			hmutex dataMutex;
			harray<Host> hosts;
			harray<unsigned short> ports;
			harray<hstream*> streams;

			~UdpReceiver()
			{
				hmutex::ScopeLock lock(&this->dataMutex);
				this->hosts.clear();
				this->ports.clear();
				foreach (hstream*, it, this->streams)
				{
					delete (*it);
				}
				this->streams.clear();
			}

		};

		StreamSocket^ sSock;
		DatagramSocket^ dSock;
		StreamSocketListener^ sServer;
		harray<StreamSocket^> _acceptedSockets;
		hmutex _mutexAcceptedSockets;
		ConnectionAccepter^ connectionAccepter;
		UdpReceiver^ udpReceiver;
		IOutputStream^ udpStream;

		// needs to be public due to Microsoft design choices
	public:
		Buffer^ _receiveBuffer;
		hstream _receiveStream;
		hmutex _mutexReceiveStream;
		IAsyncOperationWithProgress<IBuffer^, unsigned int>^ _receiveAsyncOperation;
		hmutex _mutexReceiveAsyncOperation;

	private:

		static HostName^ _makeHostName(Host host);
		static hstr _resolve(chstr host, chstr serviceName, bool wantIp, bool wantPort);

		bool _setUdpHost(HostName^ hostName, unsigned short remotePort);
		bool _readStream(hstream* stream, int& maxCount, hmutex* mutex, IInputStream^ inputStream);

		static bool _awaitAsync(State& state, hmutex::ScopeLock& lock, hmutex* mutex);
		static void _awaitAsyncCancel(State& state, hmutex::ScopeLock& lock, hmutex* mutex);
#endif

		bool _setNonBlocking(bool value);

		static bool _printLastError(chstr basicMessage, int code = 0);

	};

}
#endif
