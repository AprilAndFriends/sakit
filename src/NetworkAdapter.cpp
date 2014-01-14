/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Host.h"
#include "NetworkAdapter.h"

namespace sakit
{
	NetworkAdapter::NetworkAdapter(int comboIndex, int index, chstr name, chstr description, chstr type, Host address, Host mask, Host gateway)
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

	Host NetworkAdapter::getBroadcastIp()
	{
		if (!this->address.isIp())
		{
			return Host("255.255.255.255");
		}
		if (!this->mask.isIp())
		{
			return Host("255.255.255.255");
		}
		harray<int> numerics = this->address.getAddress().split('.').cast<int>();
		harray<int> maskNumerics = this->mask.getAddress().split('.').cast<int>();
		for_iter (i, 0, 4)
		{
			numerics[i] = ((numerics[i] & maskNumerics[i]) | (~maskNumerics[i])) & 0xFF;
		}
		return Host((unsigned char)numerics[0], (unsigned char)numerics[1], (unsigned char)numerics[2], (unsigned char)numerics[3]);
	}
	
}
