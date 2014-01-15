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
#include <hltypes/hstream.h>
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

	bool HttpResponse::parseFromRaw()
	{
		// TODOsock - this should probably be implemented using streams only
		unsigned long position = this->Raw.position();
		this->Raw.rewind();
		hstr raw = this->Raw.read();
		this->Raw.seek(position, hstream::START);
		raw.replace("\r", "");
		harray<hstr> data = raw.split("\n\n", 1, true);
		if (data.size() == 0)
		{
			return false;
		}
		if (data.size() == 2)
		{
			this->Body = data[1];
		}
		data = data[0].split('\n', 1, true);
		if (data.size() == 0)
		{
			return false;
		}
		int index = data[0].find(' ');
		if (index >= 0)
		{
			this->Protocol = data[0](0, index);
			data[0] = data[0](index + 1, -1);
			index = data[0].find(' ');
			if (index >= 0)
			{
				this->StatusCode = (HttpResponse::Code)(int)data[0](0, index);
				this->StatusMessage = data[0](index + 1, -1);
			}
			else
			{
				this->StatusMessage = data[0];
			}
		}
		this->Headers.clear();
		if (data.size() > 1)
		{
			harray<hstr> lines = data[1].split('\n');
			foreach (hstr, it, lines)
			{
				data = (*it).split(':', 1, true);
				if (data.size() > 1)
				{
					this->Headers[data[0]] = data[1](1, -1); // because there's that space character
				}
				else
				{
					this->Headers[data[0]] = "";
				}
			}
		}
		return true;
	}

}
