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
#include "Url.h"

#define HTTP_SCHEME "http://"

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

	Url::Url(chstr url)
	{
		if (url == "")
		{
			hlog::warn(sakit::logTag, "URL cannot be empty!");
			return;
		}
		hstr newUrl = url;
		if (url.starts_with(HTTP_SCHEME))
		{
			newUrl = newUrl(strlen(HTTP_SCHEME), -1);
		}
		this->host = newUrl;
		int index = 0;
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
			index = newUrl.find(chars.remove_first());
			next = parts.remove_first();
			if (index >= 0)
			{
				*current = newUrl(0, index);
				*next = newUrl = newUrl(index + 1, -1);
				if (!_checkReserved(*current))
				{
					hlog::warn(sakit::logTag, "Malformed URL: " + url);
					return;
				}
				current = next;
			}
		}
		if (!_checkReserved(this->host))
		{
			hlog::warn(sakit::logTag, "Malformed URL: " + url);
			return;
		}
		if (query != "")
		{
			this->query = Url::decodeWwwForm(query);
		}
		if (this->path != "")
		{
			this->path = "/" + Url::_decodeWwwFormComponent(this->path);
		}
		if (this->fragment != "")
		{
			this->fragment = Url::_decodeWwwFormComponent(this->fragment);
		}
	}

	Url::Url(chstr host, chstr path, hmap<hstr, hstr> query, chstr fragment)
	{
		this->host = host;
		this->path = path;
		this->query = query;
		this->fragment = fragment;
	}

	Url::~Url()
	{
	}

	bool Url::isAbsolute()
	{
		return (this->host != "");
	}

	hstr Url::toString()
	{
		hstr result;
		result += this->toRequest();
		hstr query = Url::encodeWwwForm(this->query);
		if (query != "")
		{
			result += "?" + query;
		}
		if (this->fragment != "")
		{
			result += "#" + Url::_encodeWwwFormComponent(this->fragment);
		}
		return result;
	}

	hstr Url::toRequest()
	{
		return (HTTP_SCHEME + this->host + (this->path == "" ? hstr("") : "/" + Url::_encodeWwwFormComponent(this->path)));
	}

	hstr Url::_encodeWwwFormComponent(chstr string)
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

	hstr Url::_decodeWwwFormComponent(chstr string)
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

	hstr Url::encodeWwwForm(hmap<hstr, hstr> query)
	{
		harray<hstr> parameters;
		foreach_m (hstr, it, query)
		{
			parameters += Url::_encodeWwwFormComponent(it->first) + "=" + Url::_encodeWwwFormComponent(it->second);
		}
		return parameters.join('&');
	}

	hmap<hstr, hstr> Url::decodeWwwForm(chstr string)
	{
		hmap<hstr, hstr> result;
		harray<hstr> parameters = (string.contains('&') ? string.split('&') : string.split(';'));
		harray<hstr> data;
		foreach (hstr, it, parameters)
		{
			data = (*it).split('=', 1);
			if (data.size() == 2)
			{
				result[Url::_decodeWwwFormComponent(data[0])] = Url::_decodeWwwFormComponent(data[1]);
			}
			else
			{
				result[Url::_decodeWwwFormComponent(*it)] = "";
			}
		}
		return result;
	}

}
