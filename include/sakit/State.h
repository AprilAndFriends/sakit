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
/// Defines states used internally.

#ifndef SAKIT_STATE_H
#define SAKIT_STATE_H

namespace sakit
{
	enum State
	{
		IDLE,
		BINDING,
		BOUND,
		UNBINDING,
		CONNECTING,
		CONNECTED,
		DISCONNECTING,
		RUNNING,

		FINISHED,
		FAILED
	};

}
#endif
