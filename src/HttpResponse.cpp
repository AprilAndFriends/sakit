/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "HttpResponse.h"
#include "sakit.h"

#define HTTP_DELIMITER "\r\n"

namespace sakit
{
	static int _findSequence(hstream& stream, unsigned char* sequence, int sequenceLength)
	{
		if (sequence != NULL && sequenceLength > 0)
		{
			int position = (int)stream.position();
			int size = (int)stream.size();
			int j = 0;
			for_iter (i, position, size)
			{
				if (stream[i] == sequence[j])
				{
					++j;
				}
				else
				{
					j = 0;
				}
				if (j == sequenceLength)
				{
					return i - position - 1;
				}
			}
		}
		return -1;
	}

	HttpResponse::HttpResponse() : StatusCode(UNDEFINED), HeadersComplete(false), BodyComplete(false), chunkSize(0), chunkRead(0)
	{
		this->clear();
	}

	HttpResponse::~HttpResponse()
	{
	}

	void HttpResponse::clear()
	{
		this->Protocol = "";
		this->StatusCode = UNDEFINED;
		this->StatusMessage = "";
		this->Headers.clear();
		this->Body.clear();
		this->Raw.clear();
		this->HeadersComplete = false;
		this->BodyComplete = false;
		this->chunkSize = 0;
		this->chunkRead = 0;
	}

	void HttpResponse::parseFromRaw()
	{
		if (!this->HeadersComplete)
		{
			this->_readHeaders();
		}
		if (this->HeadersComplete && !this->BodyComplete)
		{
			this->_readBody();
		}
	}

	void HttpResponse::_readHeaders()
	{
		hstr data = this->_getRawData();
		int index = 0;
		hstr line;
		harray<hstr> lineData;
		while (true)
		{
			index = data.find(HTTP_DELIMITER);
			if (index < 0)
			{
				this->Raw.seek(-data.size(), hstream::END);
				break;
			}
			line = data(0, index);
			data = data(index + 2, -1);
			if (line == "")
			{
				this->Raw.seek(-data.size(), hstream::END);
				this->HeadersComplete = true;
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

	void HttpResponse::_readBody()
	{
		if (this->Headers.try_get_by_key("Transfer-Encoding", "identity") != "chunked")
		{
			this->chunkSize = (int)this->Headers.try_get_by_key("Content-Length", "0");
			int written = this->Body.writeRaw(this->Raw);
			this->Raw.seek(written);
			this->chunkRead += written;
			if (this->chunkSize > 0 && this->chunkSize == this->chunkRead)
			{
				this->BodyComplete = true;
			}
		}
		else
		{
			int offset = 0;
			int read = 0;
			while (true)
			{
				if (this->chunkSize == 0)
				{
					offset = _findSequence(this->Raw, (unsigned char*)HTTP_DELIMITER, strlen(HTTP_DELIMITER));
					if (offset < 0)
					{
						break; // not enough bytes to read
					}
					this->chunkSize = (int)hstr(this->Raw.read(offset)).unhex();
					this->Raw.seek(2);
					if (this->chunkSize == 0)
					{
						this->Body.writeRaw(this->Raw);
						this->Raw.seek(0, hstream::END);
						this->BodyComplete = true;
						break;
					}
				}
				if (this->chunkSize > 0)
				{
					read = this->chunkSize - this->chunkRead;
					read = this->Body.writeRaw(this->Raw, read);
					this->Raw.seek(read);
					this->chunkRead += read;
					if (this->chunkRead == this->chunkSize)
					{
						this->chunkSize = 0;
						this->chunkRead = 0;
						this->Raw.seek(2);
					}
					if (this->Raw.eof())
					{
						break;
					}
				}
			}
		}
	}

	hstr HttpResponse::_getRawData()
	{
		int size = (int)(this->Raw.size() - this->Raw.position());
		char* buffer = new char[size + 1];
		this->Raw.readRaw(buffer, size);
		buffer[size] = '\0';  // classic string terminating 0-character
		hstr data = hstr(buffer);
		delete [] buffer;
		return data;
	}

	HttpResponse* HttpResponse::clone()
	{
		HttpResponse* result = new HttpResponse();
		result->Protocol = this->Protocol;
		result->StatusCode = this->StatusCode;
		result->StatusMessage = this->StatusMessage;
		result->Headers = this->Headers;
		result->Body.writeRaw((void*)&this->Body[0], (int)this->Body.size());
		result->Body.rewind();
		result->Raw.writeRaw((void*)&this->Raw[0], (int)this->Raw.size());
		result->Raw.rewind();
		result->HeadersComplete = this->HeadersComplete;
		result->BodyComplete = this->BodyComplete;
		result->chunkSize = this->chunkSize;
		result->chunkRead = this->chunkRead;
		return result;
	}

}
