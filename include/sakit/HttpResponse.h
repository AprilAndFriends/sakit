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
/// Defines an HTTP response.

#ifndef SAKIT_HTTP_RESPONSE_H
#define SAKIT_HTTP_RESPONSE_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class HttpSocketDelegate;
	class PlatformSocket;
	class TcpSocket;

	class sakitExport HttpResponse
	{
	public:
		// TODOsock - add all codes
		enum Code
		{
			UNDEFINED = 0,
			OK = 200,
			NOT_FOUND = 404
		};

		hstr Protocol;
		Code StatusCode;
		hstr StatusMessage;
		hmap<hstr, hstr> Headers;
		hstr Body;
		hstream Raw;

		HttpResponse();
		~HttpResponse();

		bool parseFromRaw();

	};

}
#endif
