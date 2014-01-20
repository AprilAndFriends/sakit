/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "sakit.h"
#include "Url.h"

#define HTTP_SCHEME "http://"

#define URL_UNRESERVED "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~"
#define URL_RESERVED ":/?#[]@"
#define URL_DELIMITERS "!$&'()*+,;="

#define HOST_ALLOWED URL_UNRESERVED URL_DELIMITERS
#define PATH_ALLOWED HOST_ALLOWED ":@"
#define QUERY_ALLOWED PATH_ALLOWED "/?"
#define FRAGMENT_ALLOWED QUERY_ALLOWED

namespace sakit
{
	Url::Url() : valid(false), port(0), queryDelimiter('&')
	{
	}

	Url::Url(chstr url) : valid(false), port(0), queryDelimiter('&')
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
				current = next;
			}
		}
		this->_checkValues(query);
	}

	Url::Url(chstr host, chstr path, hmap<hstr, hstr> query, chstr fragment) : valid(false), queryDelimiter('&')
	{
		this->host = host;
		this->path = path;
		this->query = query;
		this->fragment = fragment;
		this->_checkValues("");
	}

	void Url::_checkValues(chstr query)
	{
		int index = this->host.find(':');
		if (index >= 0)
		{
			hstr port = this->host(index + 1, -1);
			this->host = this->host(0, index);
			if (!port.is_number())
			{
				hlog::warn(sakit::logTag, "Malformed URL host: " + this->host);
				return;
			}
			unsigned int portValue = (unsigned int)port;
			if (portValue > USHRT_MAX)
			{
				hlog::warn(sakit::logTag, "Malformed URL host: " + this->host);
				return;
			}
			this->port = (unsigned short)portValue;
		}
		if (!Url::_checkCharset(this->host, HOST_ALLOWED))
		{
			hlog::warn(sakit::logTag, "Malformed URL host: " + this->host);
			return;
		}
		this->host = Url::_decodeWwwFormComponent(this->host);
		harray<hstr> paths = this->path.split('/', -1, true);
		this->path = "";
		foreach (hstr, it, paths)
		{
			if (!Url::_checkCharset((*it), PATH_ALLOWED))
			{
				hlog::warn(sakit::logTag, "Malformed URL path segment: " + (*it));
				return;
			}
			this->path += "/" + Url::_decodeWwwFormComponent(*it);
		}
		// resolving "." and ".." in path
		this->path = hdir::normalize(this->path);
		if (this->path == "." || this->path == "/.")
		{
			this->path = "";
		}
		while (true)
		{
			if (this->path.contains("/../"))
			{
				this->path = this->path.replace("/../", "/");
				continue;
			}
			if (this->path.ends_with("/.."))
			{
				this->path = this->path(0, -4);
				continue;
			}
			break;
		}
		if (query != "")
		{
			this->query = Url::decodeWwwForm(query);
		}
		if (!Url::_checkCharset(this->fragment, FRAGMENT_ALLOWED))
		{
			hlog::warn(sakit::logTag, "Malformed URL fragment: " + this->fragment);
			return;
		}
		this->fragment = Url::_decodeWwwFormComponent(this->fragment);
		this->valid = true;
	}

	Url::~Url()
	{
	}

	hstr Url::getAbsolutePath(bool withPort)
	{
		hstr result;
		result += HTTP_SCHEME + this->_encodeWwwFormComponent(this->host, HOST_ALLOWED);
		if (withPort && this->port > 0)
		{
			result += ":" + hstr(this->port);
		}
		harray<hstr> paths = this->path.split('/', -1, true);
		foreach (hstr, it, paths)
		{
			result += "/" + this->_encodeWwwFormComponent((*it), PATH_ALLOWED);
		}
		return result;
	}

	hstr Url::getBody()
	{
		hstr result;
		hstr query = Url::encodeWwwForm(this->query, this->queryDelimiter);
		if (query != "")
		{
			result += query;
		}
		if (this->fragment != "")
		{
			result += "#" + Url::_encodeWwwFormComponent(this->fragment, FRAGMENT_ALLOWED);
		}
		return result;
	}

	hstr Url::toString()
	{
		hstr result = this->getAbsolutePath(true);
		hstr body = this->getBody();
		if (body != "")
		{
			if (!body.starts_with('#'))
			{
				result += "?";
			}
			result += body;
		}
		return result;
	}

	bool Url::_checkCharset(chstr string, chstr allowed)
	{
		return (string == "" || (string.split() / (allowed + "%0123456789ABCDEFabcdef").split()).size() > 0);
	}

	hstr Url::_encodeWwwFormComponent(chstr string, chstr allowed)
	{
		hstr result;
		for_iter (i, 0, string.size())
		{
			if (allowed.contains(string[i]))
			{
				result += string[i];
			}
			else
			{
				result += hsprintf("%%%02X", string[i]);
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
		return result;
	}

	hstr Url::encodeWwwForm(hmap<hstr, hstr> query, char delimiter)
	{
		harray<hstr> parameters;
		foreach_m (hstr, it, query)
		{
			parameters += Url::_encodeWwwFormComponent(it->first, QUERY_ALLOWED) + "=" + Url::_encodeWwwFormComponent(it->second, QUERY_ALLOWED);
		}
		return parameters.join(delimiter);
	}

	hmap<hstr, hstr> Url::decodeWwwForm(chstr string, char* usedDelimiter)
	{
		char delimiter = '&';
		harray<char> delimiters = hstr(URL_DELIMITERS).split();
		foreach (char, it, delimiters)
		{
			if (string.contains(*it))
			{
				delimiter = (*it);
				break;
			}
		}
		hmap<hstr, hstr> result;
		harray<hstr> parameters = string.split(delimiter);
		harray<hstr> data;
		foreach (hstr, it, parameters)
		{
			data = (*it).split('=', 1, true);
			if (data.size() == 2)
			{
				result[Url::_decodeWwwFormComponent(data[0])] = Url::_decodeWwwFormComponent(data[1]);
			}
			else
			{
				result[Url::_decodeWwwFormComponent(*it)] = "";
			}
		}
		if (usedDelimiter != NULL)
		{
			*usedDelimiter = delimiter;
		}
		return result;
	}

}
