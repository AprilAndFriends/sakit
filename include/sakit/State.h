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

		static const harray<State> allowedBindStates;
		static const harray<State> allowedUnbindStates;
		static const harray<State> allowedConnectStates;
		static const harray<State> allowedDisconnectStates;
		static const harray<State> allowedSendStatesBasic;
		static const harray<State> allowedReceiveStatesBasic;
		static const harray<State> allowedStopReceiveStates;
		static const harray<State> allowedSetDestinationStates;
		static const harray<State> allowedJoinMulticastGroupStates;
		static const harray<State> allowedLeaveMulticastGroupStates;
		static const harray<State> allowedServerStartStates;
		static const harray<State> allowedServerStopStates;
		static const harray<State> allowedHttpExecuteStates;
		static const harray<State> allowedHttpAbortStates;

	));

}
#endif
