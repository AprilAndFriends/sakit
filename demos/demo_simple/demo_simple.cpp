/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#define LOG_TAG "demo_simple"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include <sakit/sakit.h>
#include <sakit/TcpSocket.h>
#include <sakit/TcpServer.h>

int main(int argc, char **argv)
{
	sakit::init();
	sakit::Socket* client = new sakit::TcpSocket();
	if (client->connect("localhost", 80))
	{
		hlog::write(LOG_TAG, "Connected to localhost:80");
		hstream stream;
		int received = client->receive(stream);
		hlog::writef(LOG_TAG, "Received %d bytes.", received);
		if (received > 0)
		{
			hlog::write(LOG_TAG, stream.read());
		}
	}


	sakit::destroy();
	system("pause");
	return 0;
}
