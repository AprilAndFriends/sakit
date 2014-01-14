/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "PlatformSocket.h"

namespace sakit
{
	const Host Host::Localhost("localhost");

	Host::Host()
	{
		this->address = "";
	}

	Host::Host(chstr name)
	{
		this->address = name;
	}

	Host::Host(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
	{
		this->address = hsprintf("%d.%d.%d.%d", a, b, c, d);
	}

	Host::~Host()
	{
	}

	bool Host::isIp()
	{
		harray<hstr> fragments = this->address.split('.');
		if (fragments.size() != 4)
		{
			return false;
		}
		foreach (hstr, it, fragments)
		{
			if (!(*it).is_number() || (int)(*it) >= 256)
			{
				return false;
			}
		}
		return true;
	}

	bool Host::operator==(const Host& other) const
	{
		return (this->address == other.address);
	}

	bool Host::operator!=(const Host& other) const
	{
		return (this->address !=  other.address);
	}

	hstr Host::resolveHost(chstr domain)
	{
		return PlatformSocket::resolveHost(domain);
	}

	hstr Host::resolveIp(chstr ip)
	{
		return PlatformSocket::resolveIp(ip);
	}

	hstr Host::resolveIp(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
	{
		return PlatformSocket::resolveIp(hsprintf("%d.%d.%d.%d", a, b, c, d));
	}

}
