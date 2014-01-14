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
		Host(chstr domain);
		Host(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
		~Host();

		bool isIp();

		hstr toString();

		bool operator==(const Host& other) const;
		bool operator!=(const Host& other) const;

		static const Host Localhost;

		/// @return The IP of the domain/host.
		static hstr resolveHost(chstr domain);
		/// @return The domain/host associated with this IP address.
		static hstr resolveIp(chstr ip);
		/// @return The domain/host associated with this IP address.
		static hstr resolveIp(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

	protected:
		hstr address;

	};

}
#endif
