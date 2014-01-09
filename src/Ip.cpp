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

#include "Ip.h"

namespace sakit
{
	Ip Ip::Localhost("127.0.0.1");

	Ip::Ip()
	{
		this->address = "";
	}

	Ip::Ip(chstr name)
	{
		this->address = name;
	}

	Ip::Ip(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
	{
		this->address = hsprintf("%d.%d.%d.%d", a, b, c, d);
	}

	Ip::~Ip()
	{
	}

	bool Ip::isNumeric()
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

	harray<unsigned char> Ip::getNumerics()
	{
		harray<unsigned char> numerics;
		if (this->isNumeric())
		{
			harray<hstr> fragments = this->address.split('.');
			foreach (hstr, it, fragments)
			{
				numerics += (unsigned char)(int)(*it);
			}
		}
		return numerics;
	}

	unsigned int Ip::getRawNumeric()
	{
		harray<unsigned char> numerics = this->getNumerics();
		if (numerics.size() != 4)
		{
			return 0;
		}
		return ((numerics[0] << 24) || (numerics[1] << 16) || (numerics[2] << 8) || numerics[3]);
	}

	bool Ip::operator==(const Ip& other) const
	{
		return (this->address == other.address);
	}

	bool Ip::operator!=(const Ip& other) const
	{
		return (this->address !=  other.address);
	}

}
