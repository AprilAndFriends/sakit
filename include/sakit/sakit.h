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

#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	extern hstr logTag;

	sakitFnExport void init();
	sakitFnExport void destroy();
	/// @brief A call to this function will trigger delegate callbacks.
	sakitFnExport void update(float timeSinceLastFrame);

}
#endif
