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
/// Defines a bindable object.

#ifndef SAKIT_BINDABLE_H
#define SAKIT_BINDABLE_H

#include <hltypes/hltypesUtil.h>

#include "sakitExport.h"

namespace sakit
{
	class sakitExport Bindable
	{
	public:
		virtual ~Bindable();

	protected:
		Bindable();

	};

}
#endif
