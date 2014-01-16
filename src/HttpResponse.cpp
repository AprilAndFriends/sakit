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
	HttpResponse::HttpResponse() : StatusCode(UNDEFINED), headersComplete(false), bodyComplete(false), lastChunkSize(0), lastChunkRead(0)
	{
	}

	HttpResponse::~HttpResponse()
	{
	}

	bool HttpResponse::isComplete()
	{
		return (this->headersComplete && this->bodyComplete);
	}

	bool HttpResponse::parseFromRaw()
	{
		if (this->isComplete())
		{
			return false;
		}
		hstr data = this->_getRawData();
		if (!this->headersComplete)
		{
			this->_readHeaders(data);
		}
		if (this->headersComplete)
		{
			this->_readBody(data);
		}
	}

	void HttpResponse::_readHeaders(hstr& data)
	{
		int index = 0;
		hstr line;
		harray<hstr> lineData;
		while (true)
		{
			index = data.find("\r\n");
			if (index < 0)
			{
				this->Raw.seek(-data.size(), hstream::END);
				break;
			}
			line = data(0, index);
			data = data(index + 2, -1);
			if (line == "")
			{
				this->headersComplete = true;
				break;
			}
			if (this->StatusCode == HttpResponse::UNDEFINED)
			{
				index = line.find(' ');
				if (index >= 0)
				{
					this->Protocol = line(0, index);
					line = line(index + 1, -1);
					index = line.find(' ');
					if (index >= 0)
					{
						this->StatusCode = (HttpResponse::Code)(int)line(0, index);
						this->StatusMessage = line(index + 1, -1);
					}
					else
					{
						this->StatusMessage = line;
					}
				}
			}
			else
			{
				lineData = line.split(':', 1, true);
				if (lineData.size() > 1)
				{
					this->Headers[lineData[0]] = lineData[1](1, -1); // because there's that space character
				}
				else
				{
					this->Headers[lineData[0]] = "";
				}
			}
		}
	}

	void HttpResponse::_readBody(hstr& data)
	{
		// TODOsock - implement normal reading and chunked reading
		if (this->Headers.try_get_by_key("Transfer-Encoding", "") != "chunked")
		{
		}
		else
		{
		}
	}

	hstr HttpResponse::_getRawData()
	{
		int size = (this->Raw.size() - this->Raw.position());
		char* buffer = new char[size + 1];
		this->Raw.read_raw(buffer, size);
		buffer[size + 1] = '\0';  // classic string terminating 0-character
		hstr data = hstr(buffer);
		delete [] buffer;
		return data;
	}

}
