/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a network adapter.

#ifndef SAKIT_NETWORK_ADAPTER_H
#define SAKIT_NETWORK_ADAPTER_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class sakitExport NetworkAdapter
	{
	public:
		NetworkAdapter(int comboIndex, int index, chstr name, chstr description, chstr type, Host address, Host mask, Host gateway);
		~NetworkAdapter();

		HL_DEFINE_GET(int, comboIndex, ComboIndex);
		HL_DEFINE_GET(int, index, Index);
		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(hstr, description, Description);
		HL_DEFINE_GET(hstr, type, Type);
		HL_DEFINE_GET(Host, address, Address);
		HL_DEFINE_GET(Host, mask, Mask);
		HL_DEFINE_GET(Host, gateway, Gateway);
		Host getBroadcastIp() const;

	protected:
		int comboIndex;
		int index;
		hstr name;
		hstr type;
		hstr description;
		Host address;
		Host mask;
		Host gateway;

	};

}
#endif
