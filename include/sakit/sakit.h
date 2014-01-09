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

#include "NetworkAdapter.h"
#include "sakitExport.h"

namespace sakit
{
	extern hstr logTag;

	sakitFnExport void init(int bufferSize = 65536);
	sakitFnExport void destroy();
	/// @brief A call to this function will trigger delegate callbacks.
	sakitFnExport void update(float timeSinceLastFrame);
	sakitFnExport float getRetryTimeout();
	sakitFnExport void setRetryTimeout(float value);
	sakitFnExport harray<NetworkAdapter> getNetworkAdapters();

}
#endif
