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
#include <sakit/SocketDelegate.h>
#include <sakit/TcpSocket.h>
#include <sakit/TcpServer.h>

class Sender : public sakit::SocketDelegate
{
	void onSent(sakit::Socket* socket, int bytes)
	{
		hlog::writef(LOG_TAG, "- sent %d bytes ", bytes);
	}

	void onSendFinished(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- send finished");
	}

	void onSendFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- send failed");
	}

	void onReceived(sakit::Socket* socket, hsbase* stream)
	{
		hlog::writef(LOG_TAG, "- received %d bytes ", stream->size());
	}

	void onReceiveFinished(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- receive finished");
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

	void onReceiveFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- receive failed");
		socket->disconnect();
	}

} socketDelegate;

int main(int argc, char **argv)
{
	// TODOsock - test how sockets act when sending \0
	sakit::init();
	sakit::Socket* client = new sakit::TcpSocket(&socketDelegate);
	//if (client->connect(sakit::Ip::Localhost("www.google.com"), 80))
	if (client->connect(sakit::Ip::Localhost, 54269))
	{
		hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
		hstream stream;
		stream.write("CON\t1\t1\n");
		client->send(&stream);
		client->receive(16); // receive 16 bytes max
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
