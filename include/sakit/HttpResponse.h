/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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

#define SAKIT_HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN "Access-Control-Allow-Origin"
#define SAKIT_HTTP_RESPONSE_HEADER_ACCEPT_PATCH "Accept-Patch"
#define SAKIT_HTTP_RESPONSE_HEADER_ACCEPT_RANGES "Accept-Ranges"
#define SAKIT_HTTP_RESPONSE_HEADER_AGE "Age"
#define SAKIT_HTTP_RESPONSE_HEADER_ALLOW "Allow"
#define SAKIT_HTTP_RESPONSE_HEADER_ALT_SVC "Alt-Svc"
#define SAKIT_HTTP_RESPONSE_HEADER_CACHE_CONTROLE "Cache-Control"
#define SAKIT_HTTP_RESPONSE_HEADER_CONNECTION "Connection"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_DISPOSITION "Content-Disposition"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_ENCODING "Content-Encoding"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE "Content-Language"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_LENGTH "Content-Length"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_LOCATION "Content-Location"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_MD5 "Content-MD5"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_RANGE "Content-Range"
#define SAKIT_HTTP_RESPONSE_HEADER_CONTENT_TYPE "Content-Type"
#define SAKIT_HTTP_RESPONSE_HEADER_DATE "Date"
#define SAKIT_HTTP_RESPONSE_HEADER_E_TAG "ETag"
#define SAKIT_HTTP_RESPONSE_HEADER_EXPIRES "Expires"
#define SAKIT_HTTP_RESPONSE_HEADER_LAST_MODIFIED "Last-Modified"
#define SAKIT_HTTP_RESPONSE_HEADER_LINK "Link"
#define SAKIT_HTTP_RESPONSE_HEADER_LOCATION "Location"
#define SAKIT_HTTP_RESPONSE_HEADER_P3P "P3P"
#define SAKIT_HTTP_RESPONSE_HEADER_PRAGMA "Pragma"
#define SAKIT_HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE "Proxy-Authenticate"
#define SAKIT_HTTP_RESPONSE_HEADER_PUBLIC_KEYS_PINS "Public-Key-Pins"
#define SAKIT_HTTP_RESPONSE_HEADER_REFRESH "Refresh"
#define SAKIT_HTTP_RESPONSE_HEADER_RETRY_AFTER "Retry-After"
#define SAKIT_HTTP_RESPONSE_HEADER_SERVER "Server"
#define SAKIT_HTTP_RESPONSE_HEADER_SET_COOKIE "Set-Cookie"
#define SAKIT_HTTP_RESPONSE_HEADER_STATUS "Status"
#define SAKIT_HTTP_RESPONSE_HEADER_STRICT_TRANSPORT_SECURITY "Strict-Transport-Security"
#define SAKIT_HTTP_RESPONSE_HEADER_TRAILER "Trailer"
#define SAKIT_HTTP_RESPONSE_HEADER_TRANSFER_ENCODING "Transfer-Encoding"
#define SAKIT_HTTP_RESPONSE_HEADER_TSV "TSV"
#define SAKIT_HTTP_RESPONSE_HEADER_UPGRADE "Upgrade"
#define SAKIT_HTTP_RESPONSE_HEADER_VARY "Vary"
#define SAKIT_HTTP_RESPONSE_HEADER_VIA "Via"
#define SAKIT_HTTP_RESPONSE_HEADER_WARNING "Warning"
#define SAKIT_HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE "WWW-Authenticate"
#define SAKIT_HTTP_RESPONSE_HEADER_X_FRAME_OPTIONS "X-Frame-Options"

namespace sakit
{
	class sakitExport HttpResponse
	{
	public:
		enum Code
		{
			UNDEFINED = 0, // server did not respond
			CONTINUE = 100,
			SWITCHING_PROTOCOLS = 101,
			OK = 200,
			CREATED = 201,
			ACCEPTED = 202,
			NON_AUTHORATIVE_INFORMATION = 203,
			NO_CONTENT = 204,
			RESET_CONTENT = 205,
			PARTIAL_CONTENT = 206,
			MULTIPLE_CHOICES = 300,
			MOVED_PERMANENTLY = 301,
			FOUND = 302,
			SEE_OTHER = 303,
			NOT_MODIFIED = 304,
			USE_PROXY = 305,
			TEMPORARY_REDIRECT = 307,
			BAD_REQUEST = 400,
			UNAUTHORIZED = 401,
			PAYMENT_REQUIRED = 402,
			FORBIDDEN = 403,
			NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			NOT_ACCEPTABLE = 406,
			PROXY_AUTHENTICATION_REQUIRED = 407,
			REQUEST_TIME_OUT = 408,
			CONFLICT = 409,
			GONE = 410,
			LENGTH_REQUIRED = 411,
			PRECONDITION_FAILED = 412,
			REQUEST_ENTITY_TOO_LARGE = 413,
			REQUEST_URI_TOO_LARGE = 414,
			UNSUPPORTED_MEDIA_TYPE = 415,
			REQUESTED_RANGE_NOT_SATISFIABLE = 416,
			EXPECTATION_FAILED = 417,
			INTERNAL_SERVER_ERROR = 500,
			NOT_IMPLEMENTED = 501,
			BAD_GATEWAY = 502,
			SERVICE_UNAVAILABLE = 503,
			GATEWAY_TIME_OUT = 504,
			HTTP_VERSION_NOT_SUPPORTED = 505
		};

		hstr protocol;
		Code statusCode;
		hstr statusMessage;
		hmap<hstr, hstr> headers;
		hstream body;
		hstream raw;
		bool headersComplete;
		bool bodyComplete;

		HttpResponse();
		~HttpResponse();

		void clear();
		void parseFromRaw();
		bool hasNewData();
		int consumeNewData();

		HttpResponse* clone() const;

	protected:
		int chunkSize;
		int chunkRead;
		int newDataSize;

		void _getRawData(hstr& data, int& size);
		void _readHeaders();
		void _readBody();

	};

}
#endif
