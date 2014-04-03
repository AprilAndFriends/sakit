/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "HttpSocketDelegate.h"

namespace sakit
{
	HttpSocketDelegate::HttpSocketDelegate()
	{
	}

	HttpSocketDelegate::~HttpSocketDelegate()
	{
	}

	void HttpSocketDelegate::onExecuteCompleted(HttpSocket* socket, HttpResponse* response, Url url)
	{
	}

	void HttpSocketDelegate::onExecuteFailed(HttpSocket* socket, HttpResponse* response, Url url)
	{
	}

}
