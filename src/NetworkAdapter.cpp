/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Ip.h"
#include "NetworkAdapter.h"

namespace sakit
{
	NetworkAdapter::NetworkAdapter(int comboIndex, int index, chstr name, chstr description, chstr type, Ip address, Ip mask, Ip gateway)
	{
		this->comboIndex = comboIndex;
		this->index = index;
		this->name = name;
		this->description = description;
		this->type = type;
		this->address = address;
		this->mask = mask;
		this->gateway = gateway;
	}

	NetworkAdapter::~NetworkAdapter()
	{
	}

	Ip NetworkAdapter::getBroadcastIp()
	{
		harray<unsigned char> numerics = this->address.getNumerics();
		harray<unsigned char> maskNumerics = this->mask.getNumerics();
		if (numerics.size() != 4 || maskNumerics.size() != 4)
		{
			return Ip("255.255.255.255");
		}
		for_iter (i, 0, 4)
		{
			numerics[i] = (numerics[i] & maskNumerics[i]) | (~maskNumerics[i]);
		}
		return Ip(numerics[0], numerics[1], numerics[2], numerics[3]);
	}
	
}
