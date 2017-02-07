/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a host address as either IP address or resolved domain.

#ifndef SAKIT_HOST_H
#define SAKIT_HOST_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class sakitExport Host
	{
	public:
		Host();
		Host(const char* domain);
		Host(chstr domain);
		Host(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
		~Host();

		bool isIp() const;

		hstr toString() const;

		bool operator==(const Host& other) const;
		bool operator!=(const Host& other) const;

		static const Host Localhost;
		static const Host Any;

	protected:
		hstr address;

	};

}
#endif
