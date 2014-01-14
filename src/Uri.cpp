/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "sakit.h"
#include "Uri.h"

namespace sakit
{
	/*
	gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
	sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
	*/

	static harray<hstr> reserved = hstr(":,/,?,#,[,],@").split(',');

	static bool _checkReserved(chstr string)
	{
		foreach (hstr, it, reserved)
		{
			if (string.contains(*it))
			{
				return false;
			}
		}
		return true;
	}

	Uri::Uri(chstr uri)
	{
		// TODOsock - decode www form components somewhere
		hstr newUri = uri.replace(' ', '+');
		int index = newUri.find(':');
		if (index < 0)
		{
			hlog::warn(sakit::logTag, "Malformed URI: " + uri);
			return;
		}
		this->scheme = newUri(0, index - 1);
		if (!_checkReserved(this->scheme))
		{
			hlog::warn(sakit::logTag, "Malformed URI: " + uri);
			return;
		}
		newUri = newUri(index, -1);
		if (newUri[0] != '/' || newUri[1] != '/')
		{
			hlog::warn(sakit::logTag, "Malformed URI: " + uri);
			return;
		}
		// TODOsock - finish implementation
		/*
		int pathIndex = index = newUri.find('/');
		if (index >= 0)
		{
			this->path = newUri()
		}


		newUri = newUri(2, -1);


		int pathIndex = newUri.find('/');
		int queryIndex = newUri.find('?');
		int fragmentIndex = newUri.find('#');



		bool hasPath = (pathIndex >= 0 && (queryIndex < 0 || pathIndex < queryIndex) && (fragmentIndex < 0 || pathIndex < fragmentIndex));
		bool hasQuery = (queryIndex >= 0 && (fragmentIndex < 0 || queryIndex < fragmentIndex));
		bool hasFragment = (fragmentIndex >= 0);
		if (hasPath)
		{

		}

		bool hasPath = (index >= 0);
		if (hasPath)
		{
			//this->path = newUri()
			//this->host = newUri;
			return;
		}
		this->host = newUri(0, index - 1);
		this->path = newUri = newUri(index, -1);
		index = newUri.find('?');
		hstr fragment;
		if (index >= 0)
		{
			this->path = newUri(0, index - 1);
			newUri = newUri(index, -1);

			//this->fragment = newUri(index)
			//fragments
			//this->path = this->path(index
		}
		else
		{
			index = newUri.find('#');
			if (index >= 0)
			{
				this->fragment = newUri(index)
			}
		}
		*/
	}

	Uri::Uri(chstr scheme, chstr host, chstr path, hmap<hstr, hstr> query, chstr fragment)
	{
		this->scheme = scheme;
		this->host = host;
		this->path = path;
		this->query = query;
		this->fragment = fragment;
	}

	Uri::~Uri()
	{
	}

	bool Uri::isAbsolute()
	{
		return (this->host != "");
	}

	hstr Uri::toString()
	{
		hstr result;
		result += this->scheme + ":";
		result += "//" + this->host;
		result += this->_encodeWwwFormComponent(this->path);
		harray<hstr> parameters;
		foreach_m (hstr, it, this->query)
		{
			parameters += this->_encodeWwwFormComponent(it->first) + "=" + this->_encodeWwwFormComponent(it->second);
		}
		if (parameters.size() > 0)
		{
			result += "?" + parameters.join('&');
		}
		if (this->fragment != "")
		{
			result += "#" + this->fragment;
		}
		return result;
	}

	hstr Uri::_encodeWwwFormComponent(chstr string)
	{
		hstr result;
		std::basic_string<unsigned int> chars = string.u_str();
		for_itert (unsigned int, i, 0, chars.size())
		{
			if (chars[i] < 128)
			{
				result += (char)chars[i];
				// TODOsock - encode more stuff
			}
			else if (chars[i] < 256)
			{
				result += hsprintf("%%%02X", chars[i]);
			}
			else
			{
				result += hsprintf("&#%d;", chars[i]);
			}
		}
		return result;
	}

}
