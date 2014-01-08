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

#include "sakit.h"
#include "PlatformSocket.h"

namespace sakit
{
	PlatformSocket::~PlatformSocket()
	{
		this->disconnect();
		delete [] this->receiveBuffer;
	}
	
	int PlatformSocket::_printLastError()
	{
		int code = 0;
		hstr message;
		bool print = true;
#ifdef _WIN32
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
		// TODOsock - Unix
		print = (code != EAGAIN && code != EWOULDBLOCK);
#endif
		if (print)
		{
			if (message != "")
			{
				message = hstr(code);
			}
			hlog::error(sakit::logTag, message);
		}
		return code;
	}
	
}

