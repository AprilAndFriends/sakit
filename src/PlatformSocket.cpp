/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "HttpResponse.h"
#include "PlatformSocket.h"
#include "sakit.h"

#ifndef _WIN32
#include <errno.h>
#endif

namespace sakit
{
	PlatformSocket::~PlatformSocket()
	{
		this->disconnect();
		delete [] this->receiveBuffer;
	}
	
	bool PlatformSocket::receive(HttpResponse* response, hmutex& mutex)
	{
		unsigned long position = response->Raw.position();
		response->Raw.seek(0, hstream::END);
		int count = 0;
		bool result = this->receive(&response->Raw, mutex, count);
		if (result)
		{
			response->Raw.seek(position, hstream::START);
			response->parseFromRaw();
		}
		return result;
	}

	int PlatformSocket::_printLastError(chstr basicMessage)
	{
		int code = 0;
		hstr message;
		bool print = true;
#ifdef _WIN32
#ifndef _WINRT
		// TODOsock - check if this works in WinRT
		wchar_t* buffer = L"Unknown error";
		code = WSAGetLastError();
		if (code != WSAEWOULDBLOCK)
		{
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL))
			{
				message = hstr::from_unicode((wchar_t*)buffer).split('\n').first(); // there's a \n we don't want
				LocalFree(buffer);
			}
		}
		else
		{
			print = false;
		}
#else
		// TODO
#endif
#else
		code = errno;
		print = (code != EAGAIN && code != EWOULDBLOCK);
		message = strerror(code);
#endif
		if (print)
		{
			hstr printMessage = basicMessage;
#ifndef _WINRT
			if (printMessage != "")
			{
				printMessage += ": ";
			}
			if (message == "")
			{
				message = hstr(code);
			}
#endif
			hlog::error(sakit::logTag, printMessage + message);
		}
		return code;
	}
	
}

