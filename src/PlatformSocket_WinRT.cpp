/// @file
/// @version 1.04
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WIN32) && defined(_WINRT)
#define _NO_WIN_H
#include <hltypes/hplatform.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "Base.h"
#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"
#include "UdpSocket.h"

using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;

namespace sakit
{
	extern harray<Base*> connections;
	extern hmutex connectionsMutex;
	extern int bufferSize;

	void PlatformSocket::platformInit()
	{
	}

	void PlatformSocket::platformDestroy()
	{
	}

	PlatformSocket::PlatformSocket() : connected(false), connectionLess(false), serverMode(false)
	{
		this->sSock = nullptr;
		this->dSock = nullptr;
		this->sServer = nullptr;
		this->bufferSize = sakit::bufferSize;
		this->receiveBuffer = new char[this->bufferSize];
		memset(this->receiveBuffer, 0, this->bufferSize);
	}

	bool PlatformSocket::_awaitAsync(State& result, hmutex& mutex, hmutex::ScopeLock& lock)
	{
		// TODOsock - add timeouts from settings?
		int i = 0;
		lock.acquire(&mutex);
		while (result != FINISHED)
		{
			lock.release();
			hthread::sleep(1.0f);
			++i;
			lock.acquire(&mutex);
		}
		lock.release();
		return (result != FAILED);
	}

	void PlatformSocket::_awaitAsyncCancel(State& result, hmutex& mutex, hmutex::ScopeLock& lock)
	{
		lock.acquire(&mutex);
		while (result != FINISHED)
		{
			lock.release();
			hthread::sleep(1.0f);
			lock.acquire(&mutex);
		}
		lock.release();
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

	bool PlatformSocket::setRemoteAddress(Host remoteHost, unsigned short remotePort)
	{
		return true;
	}

	bool PlatformSocket::setLocalAddress(Host localHost, unsigned short localPort)
	{
		return true;
	}

	bool PlatformSocket::tryCreateSocket()
	{
		if (this->sServer == nullptr && this->sSock == nullptr && this->dSock == nullptr)
		{
			this->connected = true;
			// create socket
			/*
			if (this->serverMode)
			{
				this->sServer = ref new StreamSocketListener();
				this->sServer->Control->QualityOfService = SocketQualityOfService::LowLatency;
			}
			else*/ if (!this->connectionLess)
			{
				this->sSock = ref new StreamSocket();
				this->sSock->Control->QualityOfService = SocketQualityOfService::LowLatency;
				this->sSock->Control->KeepAlive = true;
				this->sSock->Control->NoDelay = true;
			}
			else
			{
				this->dSock = ref new DatagramSocket();
				this->dSock->Control->QualityOfService = SocketQualityOfService::LowLatency;
				this->dSock->Control->OutboundUnicastHopLimit = 32;
				this->udpReceiver = ref new UdpReceiver();
				this->udpReceiver->socket = this;
				this->dSock->MessageReceived += ref new TypedEventHandler<DatagramSocket^, DatagramSocketMessageReceivedEventArgs^>(
					this->udpReceiver, &PlatformSocket::UdpReceiver::onReceivedDatagram);
			}
		}
		return true;
	}

	bool PlatformSocket::connect(Host remoteHost, unsigned short remotePort, Host& localHost, unsigned short& localPort, float timeout, float retryFrequency)
	{
		// TODOsock - assign local host/port if possible
		// TODOsock - implement usage of timeout and retryFrequency in this method
		if (!this->tryCreateSocket())
		{
			return false;
		}
		// create host info
		HostName^ hostName = PlatformSocket::_makeHostName(remoteHost);
		if (hostName == nullptr)
		{
			this->disconnect();
			return false;
		}
		bool _asyncResult = false;
		hmutex _mutex;
		hmutex::ScopeLock _lock;
		if (this->sSock != nullptr)
		{
			// open socket
			State _asyncState = RUNNING;
			IAsyncAction^ action = nullptr;
			try
			{
				action = this->sSock->ConnectAsync(hostName, _HL_HSTR_TO_PSTR(hstr(remotePort)), SocketProtectionLevel::PlainSocket);
				action->Completed = ref new AsyncActionCompletedHandler([&_asyncResult, &_asyncState, &_mutex](IAsyncAction^ action, AsyncStatus status)
				{
					hmutex::ScopeLock _lock(&_mutex);
					if (_asyncState == RUNNING)
					{
						if (status == AsyncStatus::Completed)
						{
							_asyncResult = true;
						}
					}
					_asyncState = FINISHED;
				});
				if (!PlatformSocket::_awaitAsync(_asyncState, _mutex, _lock))
				{
					action->Cancel();
					PlatformSocket::_awaitAsyncCancel(_asyncState, _mutex, _lock);
					this->disconnect();
					return false;
				}
			}
			catch (Platform::Exception^ e)
			{
				PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
				return false;
			}
		}
		if (this->dSock != nullptr)
		{
			_asyncResult = this->_setUdpHost(hostName, remotePort);
		}
		if (!_asyncResult)
		{
			this->disconnect();
		}
		return _asyncResult;
	}

	bool PlatformSocket::_setUdpHost(HostName^ hostName, unsigned short remotePort)
	{
		// open socket
		bool _asyncResult = false;
		State _asyncState = RUNNING;
		hmutex _mutex;
		hmutex::ScopeLock _lock;
		IAsyncOperation<IOutputStream^>^ operation = nullptr;
		try
		{
			operation = this->dSock->GetOutputStreamAsync(hostName, _HL_HSTR_TO_PSTR(hstr(remotePort)));
			operation->Completed = ref new AsyncOperationCompletedHandler<IOutputStream^>(
				[this, &_asyncResult, &_asyncState, &_mutex](IAsyncOperation<IOutputStream^>^ operation, AsyncStatus status)
			{
				hmutex::ScopeLock _lock(&_mutex);
				if (_asyncState == RUNNING)
				{
					if (status == AsyncStatus::Completed)
					{
						this->udpStream = operation->GetResults();
						_asyncResult = true;
					}
				}
				_asyncState = FINISHED;
			});
			if (!PlatformSocket::_awaitAsync(_asyncState, _mutex, _lock))
			{
				operation->Cancel();
				PlatformSocket::_awaitAsyncCancel(_asyncState, _mutex, _lock);
				return false;
			}
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return false;
		}
		return _asyncResult;
	}

	bool PlatformSocket::bind(Host localHost, unsigned short& localPort)
	{
		if (!this->tryCreateSocket())
		{
			return false;
		}
		// create host info
		HostName^ hostName = nullptr;
		if (localHost != Host::Any)
		{
			hostName = PlatformSocket::_makeHostName(localHost);
			if (hostName == nullptr)
			{
				this->disconnect();
				return false;
			}
		}
		/*
		if (this->sServer != nullptr)
		{
			// thsi isn't actually supported on WinRT
			this->connectionAccepter = ref new ConnectionAccepter();
			this->connectionAccepter->socket = this;
			this->sServer->ConnectionReceived += ref new TypedEventHandler<StreamSocketListener^, StreamSocketListenerConnectionReceivedEventArgs^>(
				this->connectionAccepter, &PlatformSocket::ConnectionAccepter::onConnectedStream);
		}
		*/
		bool _asyncResult = false;
		State _asyncState = RUNNING;
		hmutex _mutex;
		hmutex::ScopeLock _lock;
		try
		{
			IAsyncAction^ action = nullptr;
			/*
			if (this->sServer != nullptr)
			{
				action = this->sServer->BindEndpointAsync(hostName, _HL_HSTR_TO_PSTR(hstr(localPort)));
			}
			else*/ if (hostName != nullptr)
			{
				action = this->dSock->BindEndpointAsync(hostName, _HL_HSTR_TO_PSTR(hstr(localPort)));
			}
			else
			{
				action = this->dSock->BindServiceNameAsync(_HL_HSTR_TO_PSTR(hstr(localPort)));
			}
			action->Completed = ref new AsyncActionCompletedHandler([&_asyncResult, &_asyncState, &_mutex](IAsyncAction^ action, AsyncStatus status)
			{
				hmutex::ScopeLock _lock(&_mutex);
				if (_asyncState == RUNNING)
				{
					if (status == AsyncStatus::Completed)
					{
						_asyncResult = true;
					}
				}
				_asyncState = FINISHED;
			});
			if (!PlatformSocket::_awaitAsync(_asyncState, _mutex, _lock))
			{
				action->Cancel();
				PlatformSocket::_awaitAsyncCancel(_asyncState, _mutex, _lock);
				this->disconnect();
				return false;
			}
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return false;
		}
		if (!_asyncResult)
		{
			this->disconnect();
		}
		return _asyncResult;
	}

	bool PlatformSocket::joinMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		// create host info
		HostName^ groupAddressName = PlatformSocket::_makeHostName(groupAddress);
		if (groupAddressName == nullptr)
		{
			this->disconnect();
			return false;
		}
		this->dSock->JoinMulticastGroup(groupAddressName);
		return true;
	}

	bool PlatformSocket::leaveMulticastGroup(Host interfaceHost, Host groupAddress)
	{
		hlog::error(sakit::logTag, "It is not possible to leave multicast groups on WinRT!");
		return false;
	}

	bool PlatformSocket::setNagleAlgorithmActive(bool value)
	{
		if (this->sSock != nullptr)
		{
			this->sSock->Control->NoDelay = (!value);
			return true;
		}
		return false;
	}

	bool PlatformSocket::setMulticastInterface(Host address)
	{
		hlog::warn(sakit::logTag, "WinRT does not support setting the multicast interface!");
		return false;
	}

	bool PlatformSocket::setMulticastTtl(int value)
	{
		this->dSock->Control->OutboundUnicastHopLimit = value;
		return true;
	}

	bool PlatformSocket::setMulticastLoopback(bool value)
	{
		hlog::warn(sakit::logTag, "WinRT does not support changing the multicast loopback (it's always disabled)!");
		return false;
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
		hmutex::ScopeLock _lock(&this->_mutexAcceptedSockets);
		foreach (StreamSocket^, it, this->_acceptedSockets)
		{
			delete (*it);
		}
		this->_acceptedSockets.clear();
		_lock.release();
		this->connectionAccepter = nullptr;
		this->udpReceiver = nullptr;
		bool previouslyConnected = this->connected;
		this->connected = false;
		return previouslyConnected;
	}

	bool PlatformSocket::send(hstream* stream, int& count, int& sent)
	{
		bool _asyncResult = false;
		State _asyncState = RUNNING;
		hmutex _mutex;
		hmutex::ScopeLock _lock;
		int _asyncResultSize = 0;
		unsigned char* data = (unsigned char*)&(*stream)[(int)stream->position()];
		int size = hmin((int)(stream->size() - stream->position()), count);
		DataWriter^ writer = ref new DataWriter();
		writer->WriteBytes(ref new Platform::Array<unsigned char>(data, size));
		IAsyncOperationWithProgress<unsigned int, unsigned int>^ operation = nullptr;
		try
		{
			if (this->sSock != nullptr)
			{
				operation = this->sSock->OutputStream->WriteAsync(writer->DetachBuffer());
			}
			if (this->dSock != nullptr)
			{
				operation = this->udpStream->WriteAsync(writer->DetachBuffer());
			}
			operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<unsigned int, unsigned int>(
				[&_asyncResult, &_asyncState, &_asyncResultSize, &_mutex](IAsyncOperationWithProgress<unsigned int, unsigned int>^ operation, AsyncStatus status)
			{
				hmutex::ScopeLock _lock(&_mutex);
				if (_asyncState == RUNNING)
				{
					if (status == AsyncStatus::Completed)
					{
						_asyncResult = true;
						_asyncResultSize = operation->GetResults();
					}
				}
				_asyncState = FINISHED;
			});
			if (!PlatformSocket::_awaitAsync(_asyncState, _mutex, _lock))
			{
				operation->Cancel();
				PlatformSocket::_awaitAsyncCancel(_asyncState, _mutex, _lock);
				return false;
			}
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return false;
		}
		if (_asyncResultSize > 0)
		{
			stream->seek(_asyncResultSize);
			sent += _asyncResultSize;
			count -= _asyncResultSize;
			return true;
		}
		return _asyncResult;
	}

	bool PlatformSocket::receive(hstream* stream, hmutex& mutex, int& count)
	{
		if (this->sSock != nullptr)
		{
			return this->_readStream(stream, mutex, count, this->sSock->InputStream);
		}
		return false;
	}

	bool PlatformSocket::listen()
	{
		hlog::error(sakit::logTag, "Server calls are not supported on WinRT due to the problematic threading and data-sharing model of WinRT.");
		return false;
	}

	bool PlatformSocket::accept(Socket* socket)
	{
		// not supported on WinRT due to broken server model
		hlog::error(sakit::logTag, "Server calls are not supported on WinRT due to the problematic threading and data-sharing model of WinRT.");
		return false;
	}

	void PlatformSocket::ConnectionAccepter::onConnectedStream(StreamSocketListener^ listener, StreamSocketListenerConnectionReceivedEventArgs^ args)
	{
		// the socket is closed after this function exits so proper server code is not possible
	}

	bool PlatformSocket::receiveFrom(hstream* stream, Host& remoteHost, unsigned short& remotePort)
	{
		hmutex::ScopeLock _lock(&this->udpReceiver->dataMutex);
		if (this->udpReceiver->streams.size() == 0)
		{
			return false;
		}
		remoteHost = this->udpReceiver->hosts.removeFirst();
		remotePort = this->udpReceiver->ports.removeFirst();
		hstream* data = this->udpReceiver->streams.removeFirst();
		_lock.release();
		if (data->size() == 0)
		{
			delete data;
			return false;
		}
		stream->writeRaw(*data);
		delete data;
		return true;
	}

	bool PlatformSocket::_readStream(hstream* stream, hmutex& mutex, int& count, IInputStream^ inputStream)
	{
		bool _asyncResult = false;
		State _asyncState = RUNNING;
		int _asyncResultSize = 0;
		int read = (count > 0 ? hmin(this->bufferSize, count) : this->bufferSize);
		Buffer^ _buffer = ref new Buffer(read);
		hmutex::ScopeLock _lock;
		try
		{
			IAsyncOperationWithProgress<IBuffer^, unsigned int>^ operation = inputStream->ReadAsync(_buffer, read, InputStreamOptions::Partial);
			operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, unsigned int>(
				[&_asyncResult, &_asyncState, &_asyncResultSize, &mutex](IAsyncOperationWithProgress<IBuffer^, unsigned int>^ operation, AsyncStatus status)
			{
				hmutex::ScopeLock _lock(&mutex);
				if (_asyncState == RUNNING)
				{
					if (status == AsyncStatus::Completed)
					{
						_asyncResult = true;
						_asyncResultSize = operation->GetResults()->Length;
					}
				}
				_asyncState = FINISHED;
			});
			if (!PlatformSocket::_awaitAsync(_asyncState, mutex, _lock))
			{
				operation->Cancel();
				PlatformSocket::_awaitAsyncCancel(_asyncState, mutex, _lock);
				//this->disconnect(); // must disconnect connection, because WinRT terminates the connection upon cancel
				if (_asyncResultSize == 0)
				{
					return true;
				}
			}
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return false;
		}
		if (_asyncResultSize > 0)
		{
			Platform::Array<unsigned char>^ _data = ref new Platform::Array<unsigned char>(_buffer->Length);
			DataReader^ reader = DataReader::FromBuffer(_buffer);
			try
			{
				reader->ReadBytes(_data);
			}
			catch (Platform::OutOfBoundsException^ e)
			{
				return false;
			}
			reader->DetachBuffer();
			_lock.acquire(&mutex);
			stream->writeRaw(_data->Data, _data->Length);
			_asyncResultSize = (int)_data->Length;
			_lock.release();
			if (count > 0) // if don't read everything
			{
				count -= _asyncResultSize;
			}
		}
		return false;
	}

	void PlatformSocket::UdpReceiver::onReceivedDatagram(DatagramSocket^ socket, DatagramSocketMessageReceivedEventArgs^ args)
	{
		hstream* stream = new hstream();
		int count = 0;
		this->socket->_readStream(stream, hmutex(), count, args->GetDataStream());
		if (stream->size() > 0)
		{
			stream->rewind();
			hmutex::ScopeLock _lock(&this->dataMutex);
			this->hosts += Host(_HL_PSTR_TO_HSTR(args->RemoteAddress->DisplayName));
			this->ports += (unsigned short)(int)_HL_PSTR_TO_HSTR(args->RemotePort);
			this->streams += stream;
		}
		else
		{
			delete stream;
		}
	}

	bool PlatformSocket::broadcast(harray<NetworkAdapter> adapters, unsigned short port, hstream* stream, int count)
	{
		IOutputStream^ udpStream = this->udpStream;
		HostName^ hostName = nullptr;
		int sent = 0;
		int size = 0;
		bool result = false;
		foreach (NetworkAdapter, it, adapters)
		{
			hostName = PlatformSocket::_makeHostName((*it).getBroadcastIp());
			if (hostName != nullptr)
			{
				this->_setUdpHost(hostName, port);
				size = count;
				this->send(stream, size, sent);
				result = true;
				hthread::sleep(1000);
			}
		}
		this->udpStream = udpStream;
		return result;
	}

	Host PlatformSocket::resolveHost(Host domain)
	{
		return Host(PlatformSocket::_resolve(domain.toString(), "0", true, false));
	}

	Host PlatformSocket::resolveIp(Host ip)
	{
		// wow, Microsoft, just wow
		hlog::warn(sakit::logTag, "WinRT does not support resolving an IP address to a host name. Attempting anyway, but don't count on it.");
		return Host(PlatformSocket::_resolve(ip.toString(), "0", false, false));
	}

	unsigned short PlatformSocket::resolveServiceName(chstr serviceName)
	{
		return (unsigned short)(int)PlatformSocket::_resolve(Host::Any.toString(), serviceName, false, true);
	}

	hstr PlatformSocket::_resolve(chstr host, chstr serviceName, bool wantIp, bool wantPort)
	{
		Windows::Networking::HostName^ hostName = nullptr;
		// create host info
		if (!wantPort)
		{
			try
			{
				hostName = ref new HostName(_HL_HSTR_TO_PSTR(host));
			}
			catch (Platform::Exception^ e)
			{
				PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
				return "";
			}
		}
		hstr result;
		State _asyncState = RUNNING;
		hmutex _mutex;
		hmutex::ScopeLock _lock;
		try
		{
			IAsyncOperation<Collections::IVectorView<EndpointPair^>^>^ operation = DatagramSocket::GetEndpointPairsAsync(hostName, _HL_HSTR_TO_PSTR(serviceName));
			operation->Completed = ref new AsyncOperationCompletedHandler<Collections::IVectorView<EndpointPair^>^>(
				[wantIp, wantPort, &result, &_asyncState, &_mutex](IAsyncOperation<Collections::IVectorView<EndpointPair^>^>^ operation, AsyncStatus status)
			{
				hmutex::ScopeLock _lock(&_mutex);
				if (_asyncState == RUNNING)
				{
					if (status == AsyncStatus::Completed)
					{
						Collections::IVectorView<EndpointPair^>^ endpointPairs = operation->GetResults();
						if (endpointPairs != nullptr && endpointPairs->Size > 0)
						{
							for (Collections::IIterator<EndpointPair^>^ it = endpointPairs->First(); it->HasCurrent; it->MoveNext())
							{
								if (it->Current->RemoteHostName != nullptr)
								{
									if (!wantPort)
									{
										if (it->Current->RemoteHostName->Type == (wantIp ? HostNameType::Ipv4 : HostNameType::DomainName))
										{
											result = _HL_PSTR_TO_HSTR(it->Current->RemoteHostName->DisplayName);
											break;
										}
									}
									else if (it->Current->RemoteHostName->Type == HostNameType::Ipv4 || it->Current->RemoteHostName->Type == HostNameType::DomainName)
									{
										result = _HL_PSTR_TO_HSTR(it->Current->RemoteServiceName);
										if (result.isNumber())
										{
											break;
										}
										result = "";
									}
								}
							}
						}
					}
				}
				_asyncState = FINISHED;
			});
			if (!PlatformSocket::_awaitAsync(_asyncState, _mutex, _lock))
			{
				operation->Cancel();
				PlatformSocket::_awaitAsyncCancel(_asyncState, _mutex, _lock);
				return "";
			}
		}
		catch (Platform::Exception^ e)
		{
			PlatformSocket::_printLastError(_HL_PSTR_TO_HSTR(e->Message));
			return "";
		}
		return result;
	}

	harray<NetworkAdapter> PlatformSocket::getNetworkAdapters()
	{
		harray<NetworkAdapter> result;
		Collections::IVectorView<HostName^>^ hostNames = NetworkInformation::GetHostNames();
		for (Collections::IIterator<HostName^>^ it = hostNames->First(); it->HasCurrent; it->MoveNext())
		{
			result += NetworkAdapter(0, 0, "", "", "", Host(_HL_PSTR_TO_HSTR(it->Current->DisplayName)), Host(), Host());
		}
		return result;
	}

}
#endif
