/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "State.h"

namespace sakit
{
	static const State _bind[1] = { State::Idle };
	static const State _unbind[1] = { State::Bound };
	static const State _connect[1] = { State::Idle };
	static const State _disconnect[1] = { State::Connected };
	static const State _send[1] = { State::Receiving };
	static const State _receive[1] = { State::Sending };
	static const State _stopReceive[2] = { State::Receiving, State::SendingReceiving };
	static const State _setDestination[1] = { State::Bound };
	static const State _joinMulticastGroup[4] = { State::Bound, State::Sending, State::Receiving, State::SendingReceiving };
	static const State _leaveMulticastGroup[4] = { State::Bound, State::Sending, State::Receiving, State::SendingReceiving };
	static const State _serverStart[1] = { State::Bound };
	static const State _serverStop[1] = { State::Running };
	static const State _httpExecute[2] = { State::Idle, State::Connected };
	static const State _httpAbort[1] = { State::Running };

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

		const harray<State> State::allowedBindStates(_bind, HL_CONST_ARRAY_SIZE(_bind));
		const harray<State> State::allowedUnbindStates(_unbind, HL_CONST_ARRAY_SIZE(_unbind));
		const harray<State> State::allowedConnectStates(_connect, HL_CONST_ARRAY_SIZE(_connect));
		const harray<State> State::allowedDisconnectStates(_disconnect, HL_CONST_ARRAY_SIZE(_disconnect));
		const harray<State> State::allowedSendStatesBasic(_send, HL_CONST_ARRAY_SIZE(_send));
		const harray<State> State::allowedReceiveStatesBasic(_receive, HL_CONST_ARRAY_SIZE(_receive));
		const harray<State> State::allowedStopReceiveStates(_stopReceive, HL_CONST_ARRAY_SIZE(_stopReceive));
		const harray<State> State::allowedSetDestinationStates(_setDestination, HL_CONST_ARRAY_SIZE(_setDestination));
		const harray<State> State::allowedJoinMulticastGroupStates(_joinMulticastGroup, HL_CONST_ARRAY_SIZE(_joinMulticastGroup));
		const harray<State> State::allowedLeaveMulticastGroupStates(_leaveMulticastGroup, HL_CONST_ARRAY_SIZE(_leaveMulticastGroup));
		const harray<State> State::allowedServerStartStates(_serverStart, HL_CONST_ARRAY_SIZE(_serverStart));
		const harray<State> State::allowedServerStopStates(_serverStop, HL_CONST_ARRAY_SIZE(_serverStop));
		const harray<State> State::allowedHttpExecuteStates(_httpExecute, HL_CONST_ARRAY_SIZE(_httpExecute));
		const harray<State> State::allowedHttpAbortStates(_httpAbort, HL_CONST_ARRAY_SIZE(_httpAbort));

	));
}
