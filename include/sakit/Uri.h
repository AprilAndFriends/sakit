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
/// Defines a URI.

#ifndef SAKIT_URI_H
#define SAKIT_URI_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class sakitExport Uri
	{
	public:
		Uri(chstr uri);
		Uri(chstr scheme, chstr host, chstr path, hmap<hstr, hstr> query = hmap<hstr, hstr>(), chstr fragment = "");
		~Uri();

		HL_DEFINE_GET(hstr, scheme, Scheme);
		HL_DEFINE_GET(hstr, host, Host);
		HL_DEFINE_GET(hstr, path, Path);
		HL_DEFINE_GET2(hmap, hstr, hstr, query, Query);
		HL_DEFINE_GET(hstr, fragment, Fragment);
		bool isAbsolute();

		hstr toString();

	protected:
		hstr scheme;
		hstr host;
		hstr path;
		hmap<hstr, hstr> query;
		hstr fragment;

		hstr _encodeWwwFormComponent(chstr string);

	/*
uri = URI("http://foo.com/posts?id=30&limit=5#time=1305298413")
#=> #<URI::HTTP:0x00000000b14880
      URL:http://foo.com/posts?id=30&limit=5#time=1305298413>
uri.scheme
#=> "http"
uri.host
#=> "foo.com"
uri.path
#=> "/posts"
uri.query
#=> "id=30&limit=5"
uri.fragment
#=> "time=1305298413"

uri.to_s
#=> "http://foo.com/posts?id=30&limit=5#time=1305298413"	*/


	};

}
#endif
