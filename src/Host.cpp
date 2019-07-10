/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "PlatformSocket.h"
#include "sakit.h"

namespace sakit
{
	const Host Host::Localhost("localhost");
	const Host Host::Any("0.0.0.0");

	Host::Host()
	{
		this->address = "";
	}

	Host::Host(const char* domain)
	{
		this->address = domain;
	}

	Host::Host(chstr domain)
	{
		this->address = domain;
	}

	Host::Host(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
	{
		this->address = hsprintf("%d.%d.%d.%d", a, b, c, d);
	}

	bool Host::isIp() const
	{
		harray<hstr> fragments = this->address.split('.');
		if (fragments.size() != 4)
		{
			return false;
		}
		foreach (hstr, it, fragments)
		{
			if (!(*it).isNumber() || (int)(*it) >= 256)
			{
				return false;
			}
		}
		return true;
	}

	hstr Host::toString() const
	{
		return this->address;
	}

	bool Host::operator==(const Host& other) const
	{
		return (this->address == other.address);
	}

	bool Host::operator!=(const Host& other) const
	{
		return (this->address != other.address);
	}

}
