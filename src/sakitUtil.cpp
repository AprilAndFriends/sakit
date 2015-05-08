/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "sakit.h"
#include "sakitUtil.h"
#include "State.h"

namespace sakit
{
	hstr _getText(State state)
	{
		switch (state)
		{
		case IDLE:				return "idle";
		case BINDING:			return "binding";
		case BOUND:				return "bound";
		case UNBINDING:			return "unbinding";
		case CONNECTING:		return "connecting";
		case CONNECTED:			return "connected";
		case DISCONNECTING:		return "disconnecting";
		case RUNNING:			return "running";
		case SENDING:			return "sending";
		case RECEIVING:			return "receiving";
		case SENDING_RECEIVING:	return "sending and receiving";
		case FINISHED:			return "finished";
		case FAILED:			return "failed";
		}
		return "";
	}

	bool _checkState(State current, harray<State> allowed, chstr action)
	{
		if (allowed.has(current))
		{
			return true;
		}
		hstr text = _getText(current);
		if (text != "")
		{
			hlog::warn(logTag, "Cannot " + action + ", " + text + "!");
			return false;
		}
		return true;

	}

}
