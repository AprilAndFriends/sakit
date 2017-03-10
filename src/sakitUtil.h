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
/// Defines utility functions used internally.

#ifndef SAKIT_UTIL_H
#define SAKIT_UTIL_H

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "sakit.h"
#include "State.h"

namespace sakit
{
	inline bool _checkState(State current, const harray<State>& allowed, chstr action)
	{
		if (!sakit::isInitialized())
		{
			hlog::error(logTag, "SAKit is not initialized!");
			return false;
		}
		if (allowed.has(current))
		{
			return true;
		}
		hlog::warnf(logTag, "Cannot %s, current state is: %s", action.cStr(), current.getName().cStr());
		return false;
	}

}
#endif
