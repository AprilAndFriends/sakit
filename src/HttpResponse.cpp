/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "HttpResponse.h"
#include "sakit.h"

namespace sakit
{
	HttpResponse::HttpResponse() : StatusCode(UNDEFINED)
	{
	}

	HttpResponse::~HttpResponse()
	{
	}

}
