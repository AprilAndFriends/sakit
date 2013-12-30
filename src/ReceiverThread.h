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
/// Defines a thread for receiving data.

#ifndef SAKIT_RECEIVER_THREAD_H
#define SAKIT_RECEIVER_THREAD_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hthread.h>

#include "sakitExport.h"

namespace sakit
{
	class PlatformSocket;
	class ReceiverDelegate;
	class Socket;

	class sakitExport ReceiverThread : public hthread
	{
	public:
		friend class Socket;

		ReceiverThread(PlatformSocket* socket);
		~ReceiverThread();

	protected:
		PlatformSocket* socket;
		int maxBytes;
		hstream* stream;
		hmutex mutex;
		bool hasError;

		static void process(hthread*);

	};

}
#endif
