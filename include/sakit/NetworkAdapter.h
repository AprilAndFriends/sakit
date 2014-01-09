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
/// Defines a network adapter.

#ifndef SAKIT_NETWORK_ADAPTER_H
#define SAKIT_NETWORK_ADAPTER_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Ip.h"
#include "sakitExport.h"

namespace sakit
{
	class sakitExport NetworkAdapter
	{
	public:
		NetworkAdapter(int comboIndex, int index, chstr name, chstr description, chstr type, Ip address, Ip mask, Ip gateway);
		~NetworkAdapter();

		HL_DEFINE_GET(int, comboIndex, ComboIndex);
		HL_DEFINE_GET(int, index, Index);
		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(hstr, description, Description);
		HL_DEFINE_GET(hstr, type, Type);
		HL_DEFINE_GET(Ip, address, Address);
		HL_DEFINE_GET(Ip, mask, Mask);
		HL_DEFINE_GET(Ip, gateway, Gateway);
		Ip getBroadcastIp();

	protected:
		int comboIndex;
		int index;
		hstr name;
		hstr type;
		hstr description;
		Ip address;
		Ip mask;
		Ip gateway;

	};

}
#endif
