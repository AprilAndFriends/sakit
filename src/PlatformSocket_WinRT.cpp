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

#include "PlatformSocket.h"

using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;

namespace sakit
{
	// TODOsock - implement and test this properly
	void PlatformSocket::platformInit()
	{
	}

	void PlatformSocket::platformDestroy()
	{
	}

	PlatformSocket::PlatformSocket()
	{
		this->connected = false;
		this->sock = nullptr;
		this->hostName = nullptr;
		this->sendBuffer = new char[bufferSize];
		memset(this->sendBuffer, 0, bufferSize);
		this->receiveBuffer = new char[bufferSize];
		memset(this->receiveBuffer, 0, bufferSize);
	}

	bool PlatformSocket::connect(chstr host, unsigned int port)
	{
		// create host info
		try
		{
			this->hostName = ref new HostName(_HL_HSTR_TO_PSTR(host.split("/", 1).first()));
		}
		catch (Platform::Exception^ e)
		{
			hlog::error(sakit::logTag, _HL_PSTR_TO_HSTR(e->Message));
			return false;
		}
		// create socket
		this->sock = ref new StreamSocket();
		this->sock->Control->KeepAlive = false; // TODOsock - check if this is ok
		this->sock->Control->NoDelay = true;
		// open socket
		this->_asyncProcessing = true;
		this->_asyncFinished = false;
		try
		{
			IAsyncAction^ action = this->sock->ConnectAsync(this->hostName, _HL_HSTR_TO_PSTR(hstr(PORT)), SocketProtectionLevel::PlainSocket);
			action->Completed = ref new AsyncActionCompletedHandler([this](IAsyncAction^ a, AsyncStatus status)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncFinished = true;
				}
				this->_asyncProcessing = false;
			});
			// TODO - use proper mutex here
			if (!this->_awaitAsync())
			{
				return false;
			}
		}
		catch (Platform::Exception^ e)
		{
			hlog::error(System::logTag, _HL_PSTR_TO_HSTR(e->Message));
			return false;
		}
		return true;
	}

	void PlatformSocket::disconnect()
	{
		this->hostName = nullptr;
		if (this->sock != nullptr)
		{
			delete this->sock; // deleting the socket is the documented way in WinRT to close the socket in C++
			this->sock = nullptr;
		}
	}
	
	void PlatformSocket::receive(hsbase& stream, unsigned int maxBytes)
	{
		/*
		this->_asyncProcessing = true;
		this->_asyncFinished = false;
		int received = 0;
		try
		{
			Buffer^ _buffer = ref new Buffer(HTTP_BUFFER_SIZE - 1);
			IAsyncOperationWithProgress<IBuffer^, unsigned int>^ operation = this->sock->InputStream->ReadAsync(
				_buffer, HTTP_BUFFER_SIZE - 1, InputStreamOptions::None);
			operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, unsigned int>(
				[this](IAsyncOperationWithProgress<IBuffer^, unsigned int>^ operation, AsyncStatus status)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncFinished = true;
				}
				this->_asyncProcessing = false;
			});
			if (!this->_awaitAsync())
			{
				return 0;
			}
			Platform::Array<unsigned char>^ _data = ref new Platform::Array<unsigned char>(_buffer->Length);
			try
			{
				DataReader::FromBuffer(_buffer)->ReadBytes(_data);
			}
			catch (Platform::OutOfBoundsException^ e)
			{
				return 0;
			}
			memcpy(buffer, _data->Data, _data->Length);
			received = (int)_data->Length;
		}
		catch (Platform::Exception^ e)
		{
			hlog::error(System::logTag, _HL_PSTR_TO_HSTR(e->Message));
			return 0;
		}
		*/
		}
	}

	bool PlatformSocket::send(hsbase* stream)
	{
		/*
		this->_asyncProcessing = true;
		this->_asyncFinished = false;
		this->_asyncSize = 0;
		try
		{
			DataWriter^ writer = ref new DataWriter();
			writer->WriteBytes(ref new Platform::Array<unsigned char>((unsigned char*)message.c_str(), message.size()));
			IAsyncOperationWithProgress<unsigned int, unsigned int>^ operation = this->sock->OutputStream->WriteAsync(writer->DetachBuffer());
			operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<unsigned int, unsigned int>(
				[this](IAsyncOperationWithProgress<unsigned int, unsigned int>^ operation, AsyncStatus status)
			{
				if (status == AsyncStatus::Completed)
				{
					this->_asyncSize = operation->GetResults();
					this->_asyncFinished = true;
				}
				this->_asyncProcessing = false;
			});
			if (!this->_awaitAsync())
			{
				return 0;
			}
		}
		catch (Platform::Exception^ e)
		{
			hlog::error(System::logTag, _HL_PSTR_TO_HSTR(e->Message));
			return 0;
		}
		return true;
		*/
		return false;
	}

}
#endif
