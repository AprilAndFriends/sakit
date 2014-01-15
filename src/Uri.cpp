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

	static harray<char> reserved = hstr(":/?#[]@").split();

	static bool _checkReserved(chstr string)
	{
		foreach (char, it, reserved)
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
		hstr newUri = uri;
		int index = newUri.find(':');
		if (index < 0)
		{
			hlog::warn(sakit::logTag, "Malformed URI: " + uri);
			return;
		}
		this->scheme = newUri(0, index);
		if (!_checkReserved(this->scheme))
		{
			hlog::warn(sakit::logTag, "Malformed URI: " + uri);
			return;
		}
		if (this->scheme != "http")
		{
			hlog::warn(sakit::logTag, "No support for URI schemes other than HTTP!");
			return;
		}
		newUri = newUri(index + 1, -1);
		if (newUri[0] != '/' || newUri[1] != '/')
		{
			hlog::warn(sakit::logTag, "Malformed URI: " + uri);
			return;
		}
		newUri = newUri(2, -1);
		hstr query;
		hstr* current = &this->host;
		hstr* next = NULL;
		harray<hstr*> parts;
		parts += &this->path;
		parts += &query;
		parts += &this->fragment;
		harray<char> chars;
		chars += '/';
		chars += '?';
		chars += '#';
		while (parts.size() > 0)
		{
			index = newUri.find(chars.remove_first());
			next = parts.remove_first();
			if (index >= 0)
			{
				*current = newUri(0, index);
				*next = newUri = newUri(index + 1, -1);
				if (!_checkReserved(*current))
				{
					hlog::warn(sakit::logTag, "Malformed URI: " + uri);
					return;
				}
				current = next;
			}
		}
		if (query != "")
		{
			this->query = Uri::decodeWwwForm(query);
		}
		if (this->path != "")
		{
			this->path = "/" + Uri::_decodeWwwFormComponent(this->path);
		}
		if (this->fragment != "")
		{
			this->fragment = Uri::_decodeWwwFormComponent(this->fragment);
		}
	}

	Uri::Uri(chstr scheme, chstr host, chstr path, hmap<hstr, hstr> query, chstr fragment)
	{
		this->scheme = scheme;
		if (this->scheme != "http")
		{
			hlog::warn(sakit::logTag, "No support for URI schemes other than HTTP!");
			return;
		}
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
		result += Uri::_encodeWwwFormComponent(this->path);
		hstr query = Uri::encodeWwwForm(this->query);
		if (query != "")
		{
			result += "?" + query;
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
			if (chars[i] < 128 && !reserved.contains(chars[i]))
			{
				result += (char)chars[i];
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

	hstr Uri::_decodeWwwFormComponent(chstr string)
	{
		hstr current = string;
		hstr result;
		int index = 0;
		while (true)
		{
			index = current.find('%');
			if (index < 0)
			{
				result += current;
				break;
			}
			result += current(0, index);
			result += (char)current(index + 1, 2).unhex();
			current = current(index + 3, -1);
		}
		return result.replace('+', ' ');
	}

	hstr Uri::encodeWwwForm(hmap<hstr, hstr> query)
	{
		harray<hstr> parameters;
		foreach_m (hstr, it, query)
		{
			parameters += Uri::_encodeWwwFormComponent(it->first) + "=" + Uri::_encodeWwwFormComponent(it->second);
		}
		return parameters.join('&');
	}

	hmap<hstr, hstr> Uri::decodeWwwForm(chstr string)
	{
		hmap<hstr, hstr> result;
		harray<hstr> parameters = (string.contains('&') ? string.split('&') : string.split(';'));
		harray<hstr> data;
		foreach (hstr, it, parameters)
		{
			data = (*it).split('=', 1);
			if (data.size() == 2)
			{
				result[Uri::_decodeWwwFormComponent(data[0])] = Uri::_decodeWwwFormComponent(data[1]);
			}
			else
			{
				result[Uri::_decodeWwwFormComponent(*it)] = "";
			}
		}
		return result;
	}

}
