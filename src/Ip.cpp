/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Ip.h"

namespace sakit
{
	Ip Ip::Localhost("127.0.0.1");

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
	
}
