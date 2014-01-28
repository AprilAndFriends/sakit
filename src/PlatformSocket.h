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

#ifdef __APPLE__
#include <netinet/in.h>
#endif
#ifdef _WINRT
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
#endif

namespace sakit
{
	class HttpResponse;
	class Socket;

	class PlatformSocket
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
		bool receive(hstream* stream, hmutex& mutex, int& maxBytes);
		bool receive(HttpResponse* response, hmutex& mutex);
		bool receiveFrom(hstream* stream, Host& host, unsigned short& port);
		bool listen();
		bool accept(Socket* socket);

		bool joinMulticastGroup(Host host, unsigned short port, Host groupAddress);
		bool setMulticastInterface(Host address);
		bool setMulticastTtl(int value);
		bool setMulticastLoopback(bool value);
		bool setNagleAlgorithmActive(bool value);

		static bool broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count);
		static hstr resolveHost(chstr domain);
		static hstr resolveIp(chstr ip);
		static harray<NetworkAdapter> getNetworkAdapters();
		
		static void platformInit();
		static void platformDestroy();

	protected:
		bool connected;
		bool connectionLess;
		char* receiveBuffer;
		int bufferSize;

#if !defined(_WIN32) || !defined(_WINRT)
		unsigned int sock;
		struct addrinfo* info;
		struct sockaddr_storage* address;
		struct sockaddr_in* multicastGroupAddress;

		bool _checkReceivedBytes(unsigned long* received);
		bool _checkResult(int result, chstr functionName, bool disconnectOnError = true);
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
				this->dataMutex.lock();
				this->hosts.clear();
				this->ports.clear();
				foreach (hstream*, it, this->streams)
				{
					delete (*it);
				}
				this->streams.clear();
				this->dataMutex.unlock();
			}

		};

		StreamSocket^ sSock;
		DatagramSocket^ dSock;
		StreamSocketListener^ sServer;
		bool _server;
		bool _asyncConnected;
		Result _asyncConnectionResult;
		hmutex _mutexConnection;
		bool _asyncBound;
		Result _asyncBindingResult;
		hmutex _mutexBinder;
		bool _asyncSent;
		Result _asyncSendingResult;
		int _asyncSentSize;
		hmutex _mutexSender;
		bool _asyncReceived;
		Result _asyncReceivingResult;
		int _asyncReceivedSize;
		hmutex _mutexReceiver;
		harray<StreamSocket^> _acceptedSockets;
		hmutex _mutexAcceptedSockets;
		ConnectionAccepter^ connectionAccepter;
		UdpReceiver^ udpReceiver;

		static Windows::Networking::HostName^ _makeHostName(Host host);
		static hstr _resolve(chstr host, bool wantIp);
		bool _readStream(hstream* stream, hmutex& mutex, int& count, IInputStream^ inputStream);

		static bool _awaitAsync(Result& result, hmutex& mutex);
#endif

		bool _setNonBlocking(bool value);

		static int _printLastError(chstr basicMessage);

	};

}
#endif
