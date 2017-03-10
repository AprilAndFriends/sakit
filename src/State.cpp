/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "State.h"

namespace sakit
{
	HL_ENUM_CLASS_DEFINE(State,
	(
		HL_ENUM_DEFINE(State, Idle);
		HL_ENUM_DEFINE(State, Binding);
		HL_ENUM_DEFINE(State, Bound);
		HL_ENUM_DEFINE(State, Unbinding);
		HL_ENUM_DEFINE(State, Connecting);
		HL_ENUM_DEFINE(State, Connected);
		HL_ENUM_DEFINE(State, Disconnecting);
		HL_ENUM_DEFINE(State, Running);
		HL_ENUM_DEFINE(State, Sending);
		HL_ENUM_DEFINE(State, Receiving);
		HL_ENUM_DEFINE(State, SendingReceiving);
		HL_ENUM_DEFINE(State, Finished);
		HL_ENUM_DEFINE(State, Failed);

		harray<State> State::allowedBindStates;
		harray<State> State::allowedUnbindStates;
		harray<State> State::allowedConnectStates;
		harray<State> State::allowedDisconnectStates;
		harray<State> State::allowedSendStatesBasic;
		harray<State> State::allowedReceiveStatesBasic;
		harray<State> State::allowedStopReceiveStates;
		harray<State> State::allowedSetDestinationStates;
		harray<State> State::allowedJoinMulticastGroupStates;
		harray<State> State::allowedLeaveMulticastGroupStates;
		harray<State> State::allowedServerStartStates;
		harray<State> State::allowedServerStopStates;
		harray<State> State::allowedHttpExecuteStates;
		harray<State> State::allowedHttpAbortStates;

	));
}
