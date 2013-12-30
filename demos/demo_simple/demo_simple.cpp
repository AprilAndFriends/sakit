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
#include <hltypes/hthread.h>

#include <sakit/sakit.h>
#include <sakit/ReceiverDelegate.h>
#include <sakit/TcpSocket.h>
#include <sakit/TcpServer.h>

class Receiver : public sakit::ReceiverDelegate
{
	void onReceived(sakit::Socket* socket, hsbase* stream)
	{
		hlog::writef(LOG_TAG, "- received %d bytes ", stream->size());
	}

	void onFinished(sakit::Socket* socket)
	{
		socket->disconnect();
	}

	void onFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- failed");
		socket->disconnect();
	}

} receiver;

int main(int argc, char **argv)
{
	sakit::init();
	sakit::Socket* client = new sakit::TcpSocket(&receiver);
	//if (client->connect(sakit::Ip::Localhost("www.google.com"), 80))
	if (client->connect(sakit::Ip::Localhost, 54269))
	{
		hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
		hstream stream;
		stream.write("CON\t1\t1\n");
		client->send(&stream);
		client->receive(6); // receive 6 bytes
		while (client->isConnected())
		{
			sakit::update(0.0f);
			hthread::sleep(100.0f);
		}
		hlog::write(LOG_TAG, "Disconnected.");
	}
	delete client;
	sakit::destroy();
	system("pause");
	return 0;
}
