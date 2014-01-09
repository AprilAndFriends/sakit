/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an IP address either by name or by direct IP.

#ifndef SAKIT_IP_H
#define SAKIT_IP_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class sakitExport Ip
	{
	public:
		Ip();
		Ip(chstr name);
		Ip(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
		~Ip();

		HL_DEFINE_GET(hstr, address, Address);
		bool isNumeric();
		harray<unsigned char> getNumerics();
		unsigned int getRawNumeric();

		bool operator==(const Ip& other) const;
		bool operator!=(const Ip& other) const;

		static Ip Localhost;

	protected:
		hstr address;

	};

}
#endif
