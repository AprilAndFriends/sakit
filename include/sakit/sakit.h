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
/// Defines global functions for init and destroy.

#ifndef SAKIT_H
#define SAKIT_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "Host.h"
#include "NetworkAdapter.h"
#include "sakitExport.h"

namespace sakit
{
	extern hstr logTag;

	sakitFnExport void init(int bufferSize = 65536);
	sakitFnExport void destroy();
	/// @brief A call to this function will trigger delegate callbacks.
	sakitFnExport void update(float timeSinceLastFrame = 0.0f);
	sakitFnExport float getRetryTimeout();
	sakitFnExport void setRetryTimeout(float value);
	sakitFnExport int getRetryAttempts();
	sakitFnExport void setRetryAttempts(int value);
	sakitFnExport harray<NetworkAdapter> getNetworkAdapters();
	/// @return The IP of the domain/host.
	sakitFnExport Host resolveHost(Host domain);
	/// @return The domain/host associated with this IP address.
	sakitFnExport Host resolveIp(Host ip);
	/// @return The port for the given service name.
	sakitFnExport unsigned short resolveServiceName(chstr serviceName);
	/// @note Encodes e.g. " into &quot;
	sakitFnExport hstr encodeHtmlEntities(chstr string);
	/// @note Decodes e.g. &quot; into "
	sakitFnExport hstr decodeHtmlEntities(chstr string);

}
#endif
