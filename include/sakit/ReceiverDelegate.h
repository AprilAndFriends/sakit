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
/// Defines a receiver delegate.

#ifndef SAKIT_RECEIVER_DELEGATE_H
#define SAKIT_RECEIVER_DELEGATE_H

#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "sakitExport.h"

namespace sakit
{
	class Socket;

	class sakitExport ReceiverDelegate
	{
	public:
		ReceiverDelegate();
		virtual ~ReceiverDelegate();

		virtual void onReceived(hsbase* stream) = 0;
		virtual void onFailure() = 0;

	};

}
#endif
