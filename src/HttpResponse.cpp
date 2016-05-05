/// @file
/// @version 1.05
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

	HttpResponse::HttpResponse() : statusCode(UNDEFINED), headersComplete(false), bodyComplete(false), chunkSize(0), chunkRead(0)
	{
		this->clear();
	}

	HttpResponse::~HttpResponse()
	{
	}

	void HttpResponse::clear()
	{
		this->protocol = "";
		this->statusCode = UNDEFINED;
		this->statusMessage = "";
		this->headers.clear();
		this->body.clear();
		this->raw.clear();
		this->headersComplete = false;
		this->bodyComplete = false;
		this->chunkSize = 0;
		this->chunkRead = 0;
	}

	void HttpResponse::parseFromRaw()
	{
		if (!this->headersComplete)
		{
			this->_readHeaders();
		}
		if (this->headersComplete && !this->bodyComplete)
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
			index = data.indexOf(HTTP_DELIMITER);
			if (index < 0)
			{
				this->raw.seek(-data.size(), hstream::END);
				break;
			}
			line = data(0, index);
			data = data(index + 2, -1);
			if (line == "")
			{
				this->raw.seek(-data.size(), hstream::END);
				this->headersComplete = true;
				break;
			}
			if (this->statusCode == HttpResponse::UNDEFINED)
			{
				index = line.indexOf(' ');
				if (index >= 0)
				{
					this->protocol = line(0, index);
					line = line(index + 1, -1);
					index = line.indexOf(' ');
					if (index >= 0)
					{
						this->statusCode = (HttpResponse::Code)(int)line(0, index);
						this->statusMessage = line(index + 1, -1);
					}
					else
					{
						this->statusMessage = line;
					}
				}
			}
			else
			{
				lineData = line.split(':', 1, true);
				if (lineData.size() > 1)
				{
					this->headers[lineData[0]] = lineData[1](1, -1); // because there's that space character
				}
				else
				{
					this->headers[lineData[0]] = "";
				}
			}
		}
	}

	void HttpResponse::_readBody()
	{
		if (this->headers.tryGet("Transfer-Encoding", "identity") != "chunked")
		{
			this->chunkSize = (int)this->headers.tryGet("Content-Length", "0");
			int written = this->body.writeRaw(this->raw);
			this->raw.seek(written);
			this->chunkRead += written;
			if (this->chunkSize > 0 && this->chunkSize == this->chunkRead)
			{
				this->bodyComplete = true;
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
					offset = _findSequence(this->raw, (unsigned char*)HTTP_DELIMITER, strlen(HTTP_DELIMITER));
					if (offset < 0)
					{
						break; // not enough bytes to read
					}
					this->chunkSize = (int)hstr(this->raw.read(offset)).unhex();
					this->raw.seek(2);
					if (this->chunkSize == 0)
					{
						this->body.writeRaw(this->raw);
						this->raw.seek(0, hstream::END);
						this->bodyComplete = true;
						break;
					}
				}
				if (this->chunkSize > 0)
				{
					read = this->chunkSize - this->chunkRead;
					read = this->body.writeRaw(this->raw, read);
					this->raw.seek(read);
					this->chunkRead += read;
					if (this->chunkRead == this->chunkSize)
					{
						this->chunkSize = 0;
						this->chunkRead = 0;
						this->raw.seek(2);
					}
					if (this->raw.eof())
					{
						break;
					}
				}
			}
		}
	}

	hstr HttpResponse::_getRawData()
	{
		int size = (int)(this->raw.size() - this->raw.position());
		char* buffer = new char[size + 1];
		this->raw.readRaw(buffer, size);
		buffer[size] = '\0';  // classic string terminating 0-character
		hstr data = hstr(buffer);
		delete [] buffer;
		return data;
	}

	HttpResponse* HttpResponse::clone()
	{
		HttpResponse* result = new HttpResponse();
		result->protocol = this->protocol;
		result->statusCode = this->statusCode;
		result->statusMessage = this->statusMessage;
		result->headers = this->headers;
		result->body = this->body; // assignment operator is properly implemented for hstream
		result->body.rewind();
		result->raw = this->raw; // assignment operator is properly implemented for hstream
		result->raw.rewind();
		result->headersComplete = this->headersComplete;
		result->bodyComplete = this->bodyComplete;
		result->chunkSize = this->chunkSize;
		result->chunkRead = this->chunkRead;
		return result;
	}

}
