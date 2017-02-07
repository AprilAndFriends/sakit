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

		UdpReceiverThread(PlatformSocket* socket, float* timeout, float* retryFrequency);
		~UdpReceiverThread();

	protected:
		harray<Host> remoteHosts;
		harray<unsigned short> remotePorts;
		harray<hstream*> streams;

		void _updateProcess();

	};

}
#endif
