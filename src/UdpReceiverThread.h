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
/// Defines a thread for receiving data through UDP.

#ifndef SAKIT_UDP_RECEIVER_THREAD_H
#define SAKIT_UDP_RECEIVER_THREAD_H

#include <hltypes/harray.h>
#include <hltypes/hstream.h>

#include "Host.h"
#include "ReceiverThread.h"

namespace sakit
{
	class PlatformSocket;
	class UdpSocket;

	class UdpReceiverThread : public ReceiverThread
	{
	public:
		friend class UdpSocket;

		UdpReceiverThread(PlatformSocket* socket);
		~UdpReceiverThread();

	protected:
		harray<Host> remoteHosts;
		harray<unsigned short> remotePorts;
		harray<hstream*> streams;

		void _updateProcess();

	};

}
#endif
