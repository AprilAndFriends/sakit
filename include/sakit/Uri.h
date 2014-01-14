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
/// Defines a URI.

#ifndef SAKIT_URI_H
#define SAKIT_URI_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class sakitExport Uri
	{
	public:
		Uri(chstr address);
		~Uri();

	};

}
#endif
