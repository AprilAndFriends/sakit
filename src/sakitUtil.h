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
/// Defines utility functions used internally.

#ifndef SAKIT_UTIL_H
#define SAKIT_UTIL_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "State.h"

namespace sakit
{
	bool _checkState(State current, harray<State> allowed, chstr action);

}
#endif
