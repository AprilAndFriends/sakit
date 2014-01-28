/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "PlatformSocket.h"
#include "ReceiverThread.h"
#include "sakit.h"
#include "SenderThread.h"
#include "SocketBase.h"

namespace sakit
{
	SocketBase::SocketBase() : Base()
	{
	}

	SocketBase::~SocketBase()
	{
	}

	int SocketBase::_send(chstr data)
	{
		hstream stream;
		stream.write(data);
		stream.rewind();
		return this->_send(&stream, stream.size());
	}

}
