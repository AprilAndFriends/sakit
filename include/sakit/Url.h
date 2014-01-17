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
/// Defines a URL. This implementation does not support the entire RFC3986 standard but only a basic HTTP scheme.

#ifndef SAKIT_URL_H
#define SAKIT_URL_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class sakitExport Url
	{
	public:
		Url();
		Url(chstr url);
		Url(chstr host, chstr path, hmap<hstr, hstr> query = hmap<hstr, hstr>(), chstr fragment = "");
		~Url();

		HL_DEFINE_IS(valid, Valid);
		HL_DEFINE_GET(hstr, host, Host);
		HL_DEFINE_GET(hstr, path, Path);
		HL_DEFINE_GET2(hmap, hstr, hstr, query, Query);
		HL_DEFINE_GET(hstr, fragment, Fragment);

		/// @note The returned value is fully encoded.
		hstr getAbsolutePath();
		/// @note The returned value is fully encoded.
		hstr getBody();
		/// @note The returned value is fully encoded.
		hstr toString();

		static hstr encodeWwwForm(hmap<hstr, hstr> query, char delimiter = '&');
		static hmap<hstr, hstr> decodeWwwForm(chstr string, char* usedDelimiter = NULL);

	protected:
		bool valid;
		hstr host;
		hstr path;
		hmap<hstr, hstr> query;
		hstr fragment;
		char queryDelimiter;

		void _checkValues(chstr query);

		static bool _checkCharset(chstr string, chstr allowed);
		static hstr _encodeWwwFormComponent(chstr string, chstr allowed);
		static hstr _decodeWwwFormComponent(chstr string);

	};

}
#endif
