/// @file
/// @version 1.2
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

#include <hltypes/henum.h>
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
		HL_ENUM_CLASS_PREFIX_DECLARE(sakitExport, Code,
		(
			HL_ENUM_DECLARE(Code, Undefined);
			HL_ENUM_DECLARE(Code, Continue);
			HL_ENUM_DECLARE(Code, SwitchingProtocols);
			HL_ENUM_DECLARE(Code, Ok);
			HL_ENUM_DECLARE(Code, Created);
			HL_ENUM_DECLARE(Code, Accepted);
			HL_ENUM_DECLARE(Code, NonAuthorativeInformation);
			HL_ENUM_DECLARE(Code, NoContent);
			HL_ENUM_DECLARE(Code, ResetContent);
			HL_ENUM_DECLARE(Code, PartialContent);
			HL_ENUM_DECLARE(Code, MultipleChoices);
			HL_ENUM_DECLARE(Code, MovedPermanently);
			HL_ENUM_DECLARE(Code, Found);
			HL_ENUM_DECLARE(Code, SeeOther);
			HL_ENUM_DECLARE(Code, NotModified);
			HL_ENUM_DECLARE(Code, UseProxy);
			HL_ENUM_DECLARE(Code, TemporaryRedirect);
			HL_ENUM_DECLARE(Code, BadRequest);
			HL_ENUM_DECLARE(Code, Unauthorized);
			HL_ENUM_DECLARE(Code, PaymentRequired);
			HL_ENUM_DECLARE(Code, Forbidden);
			HL_ENUM_DECLARE(Code, NotFound);
			HL_ENUM_DECLARE(Code, MethodNotAllowed);
			HL_ENUM_DECLARE(Code, NotAcceptable);
			HL_ENUM_DECLARE(Code, ProxyAuthenticationRequired);
			HL_ENUM_DECLARE(Code, RequestTimeOut);
			HL_ENUM_DECLARE(Code, Conflict);
			HL_ENUM_DECLARE(Code, Gone);
			HL_ENUM_DECLARE(Code, LengthRequired);
			HL_ENUM_DECLARE(Code, PreconditionFailed);
			HL_ENUM_DECLARE(Code, RequestEntityTooLarge);
			HL_ENUM_DECLARE(Code, RequestUriTooLarge);
			HL_ENUM_DECLARE(Code, UnsupportedMediaType);
			HL_ENUM_DECLARE(Code, RequestedRangeNotSatisfiable);
			HL_ENUM_DECLARE(Code, ExpectationFailed);
			HL_ENUM_DECLARE(Code, InternalServerError);
			HL_ENUM_DECLARE(Code, NotImplemented);
			HL_ENUM_DECLARE(Code, BadGateway);
			HL_ENUM_DECLARE(Code, ServiceUnavailable);
			HL_ENUM_DECLARE(Code, GatewayTimeOut);
			HL_ENUM_DECLARE(Code, HttpVersionNotSupported);
		));

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
