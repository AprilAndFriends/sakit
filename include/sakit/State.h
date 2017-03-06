/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines states used internally.

#ifndef SAKIT_STATE_H
#define SAKIT_STATE_H

#include <hltypes/henum.h>

#include "sakitExport.h"

namespace sakit
{
	HL_ENUM_CLASS_PREFIX_DECLARE(sakitExport, State,
	(
		HL_ENUM_DECLARE(State, Idle);
		HL_ENUM_DECLARE(State, Binding);
		HL_ENUM_DECLARE(State, Bound);
		HL_ENUM_DECLARE(State, Unbinding);
		HL_ENUM_DECLARE(State, Connecting);
		HL_ENUM_DECLARE(State, Connected);
		HL_ENUM_DECLARE(State, Disconnecting);
		HL_ENUM_DECLARE(State, Running);
		HL_ENUM_DECLARE(State, Sending);
		HL_ENUM_DECLARE(State, Receiving);
		HL_ENUM_DECLARE(State, SendingReceiving);
		HL_ENUM_DECLARE(State, Finished);
		HL_ENUM_DECLARE(State, Failed);

		static harray<State> allowedBindStates;
		static harray<State> allowedUnbindStates;
		static harray<State> allowedConnectStates;
		static harray<State> allowedDisconnectStates;
		static harray<State> allowedSendStatesBasic;
		static harray<State> allowedReceiveStatesBasic;
		static harray<State> allowedStopReceiveStates;
		static harray<State> allowedSetDestinationStates;
		static harray<State> allowedJoinMulticastGroupStates;
		static harray<State> allowedLeaveMulticastGroupStates;
		static harray<State> allowedServerStartStates;
		static harray<State> allowedServerStopStates;
		static harray<State> allowedHttpExecuteStates;
		static harray<State> allowedHttpAbortStates;

	));

}
#endif
