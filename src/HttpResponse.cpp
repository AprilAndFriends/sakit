/// @file
/// @version 1.1
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

	HL_ENUM_CLASS_DEFINE(HttpResponse::Code,
	(
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Undefined, 0);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Continue, 100);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, SwitchingProtocols, 101);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Ok, 200);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Created, 201);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Accepted, 202);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, NonAuthorativeInformation, 203);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, NoContent, 204);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, ResetContent, 205);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, PartialContent, 206);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, MultipleChoices, 300);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, MovedPermanently, 301);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Found, 302);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, SeeOther, 303);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, NotModified, 304);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, UseProxy, 305);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, TemporaryRedirect, 307);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, BadRequest, 400);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Unauthorized, 401);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, PaymentRequired, 402);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Forbidden, 403);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, NotFound, 404);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, MethodNotAllowed, 405);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, NotAcceptable, 406);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, ProxyAuthenticationRequired, 407);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, RequestTimeOut, 408);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Conflict, 409);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, Gone, 410);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, LengthRequired, 411);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, PreconditionFailed, 412);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, RequestEntityTooLarge, 413);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, RequestUriTooLarge, 414);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, UnsupportedMediaType, 415);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, RequestedRangeNotSatisfiable, 416);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, ExpectationFailed, 417);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, InternalServerError, 500);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, NotImplemented, 501);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, BadGateway, 502);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, ServiceUnavailable, 503);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, GatewayTimeOut, 504);
		HL_ENUM_DEFINE_VALUE(HttpResponse::Code, HttpVersionNotSupported, 505);
	));

	HttpResponse::HttpResponse() : statusCode(Code::Undefined), headersComplete(false), bodyComplete(false), chunkSize(0), chunkRead(0), newDataSize(0), body(2 * 1024 * 1024) // TEMP HACK, prevents crashes
	{
		this->clear();
	}

	HttpResponse::~HttpResponse()
	{
	}

	void HttpResponse::clear()
	{
		this->protocol = "";
		this->statusCode = Code::Undefined;
		this->statusMessage = "";
		this->headers.clear();
        this->body.clear(2 * 1024 * 1024); // TEMP HACK, prevents crashes
		this->raw.clear(2 * 1024 * 1024); // TEMP HACK, prevents crashes
		this->headersComplete = false;
		this->bodyComplete = false;
		this->chunkSize = 0;
		this->chunkRead = 0;
		this->newDataSize = 0;
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

	bool HttpResponse::hasNewData()
	{
		return (this->newDataSize > 0);
	}

	int HttpResponse::consumeNewData()
	{
		if (this->newDataSize > 0)
		{
			int result = this->newDataSize;
			this->newDataSize = 0;
			return result;
		}
		return 0;
	}

	void HttpResponse::_readHeaders()
	{
		hstr data;
		int size = 0;
		this->_getRawData(data, size);
		int index = 0;
		int read = 0;
		hstr line;
		harray<hstr> lineData;
		while (true)
		{
			index = data.indexOf(HTTP_DELIMITER);
			if (index < 0)
			{
				this->raw.seek(read - size, hstream::END);
				break;
			}
			line = (index > 0 ? data(0, index) : "");
			data = data(index + 2, -1); // +2 is HTTP_DELIMITER's size
			read += index + 2;
			if (line == "")
			{
				this->raw.seek(read - size, hstream::END);
				this->headersComplete = true;
				break;
			}
			if (this->statusCode == HttpResponse::Code::Undefined)
			{
				index = line.indexOf(' ');
				if (index >= 0)
				{
					this->protocol = line(0, index);
					line = line(index + 1, -1);
					index = line.indexOf(' ');
					if (index >= 0)
					{
						this->statusCode = HttpResponse::Code::fromInt((int)line(0, index));
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
		if (this->headers.tryGet(SAKIT_HTTP_RESPONSE_HEADER_TRANSFER_ENCODING, "identity") != "chunked")
		{
			this->chunkSize = (int)this->headers.tryGet(SAKIT_HTTP_RESPONSE_HEADER_CONTENT_LENGTH, "0");
			int written = this->body.writeRaw(this->raw);
			this->raw.seek(written);
			this->chunkRead += written;
			if (written > 0)
			{
				this->newDataSize += written;
			}
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
					this->newDataSize += read;
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

	void HttpResponse::_getRawData(hstr& data, int& size)
	{
		size = (int)(this->raw.size() - this->raw.position());
		char* buffer = new char[size + 1];
		this->raw.readRaw(buffer, size);
		buffer[size] = '\0'; // classic string terminating 0-character
		data = hstr(buffer);
		delete[] buffer;
	}

	HttpResponse* HttpResponse::clone() const
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
		result->newDataSize = this->newDataSize;
		return result;
	}

}
