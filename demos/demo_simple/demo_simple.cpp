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
#include <sakit/ServerDelegate.h>
#include <sakit/SocketDelegate.h>
#include <sakit/TcpSocket.h>
#include <sakit/TcpServer.h>

#define TEST_PORT 53334

class ServerDelegate : public sakit::ServerDelegate
{
	/*
	void onBound(sakit::Server* server)
	{
		hlog::writef(LOG_TAG, "- server bound to '%d'", server->getFullHost().c_str());
	}

	void onBindFailed(Server* server)
	{
		hlog::writef(LOG_TAG, "- server binding failed for port '%d'", server->getPort());
	}
	*/

	void onAccepted(sakit::Server* server, sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- server '%s' accepted connection '%s'",
			server->getFullHost().c_str(), socket->getFullHost().c_str());
		//socket->receive();
		//socket->send("Hello.\n");
		socket->receive(9); // receive 8 bytes max
		hstream stream;
		stream.write("Hello.");
		socket->send(&stream);
		while (socket->isConnected())
		{
			sakit::update(0.0f);
			hthread::sleep(100.0f);
		}
	}

} serverDelegate;

class SocketDelegate : public sakit::SocketDelegate
{
	void onSent(sakit::Socket* socket, int bytes)
	{
		hlog::writef(LOG_TAG, "- client sent %d bytes ", bytes);
	}

	void onSendFinished(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- client send finished");
	}

	void onSendFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- client send failed");
	}

	void onReceived(sakit::Socket* socket, hsbase* stream)
	{
		hlog::writef(LOG_TAG, "- client received %d bytes ", stream->size());
	}

	void onReceiveFinished(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- client receive finished");
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

	void onReceiveFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- client receive failed");
		socket->disconnect();
	}

} socketDelegate;

class ClientDelegate : public sakit::SocketDelegate
{
	void onSent(sakit::Socket* socket, int bytes)
	{
		hlog::writef(LOG_TAG, "- server sent %d bytes ", bytes);
	}

	void onSendFinished(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- server send finished");
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

	void onSendFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- server send failed");
		socket->disconnect();
	}

	void onReceived(sakit::Socket* socket, hsbase* stream)
	{
		hlog::writef(LOG_TAG, "- server received %d bytes ", stream->size());
	}

	void onReceiveFinished(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- server receive finished");
	}

	void onReceiveFailed(sakit::Socket* socket)
	{
		hlog::write(LOG_TAG, "- server receive failed");
	}

} clientDelegate;

int main(int argc, char **argv)
{
	// TODOsock - test how sockets act when sending \0
	sakit::init();
	sakit::Server* server = new sakit::TcpServer(&serverDelegate, &clientDelegate);
	if (server->bind(sakit::Ip::Localhost, TEST_PORT))
	{
		hlog::write(LOG_TAG, "Bound to " + server->getFullHost());
		server->start();
		sakit::Socket* client = new sakit::TcpSocket(&socketDelegate);
		if (client->connect(sakit::Ip::Localhost, TEST_PORT))
		{
			hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
			hstream stream;
			stream.write("Hi there.");
			client->send(&stream);
			client->receive(6); // receive 16 bytes max
			while (client->isConnected())
			{
				sakit::update(0.0f);
				hthread::sleep(100.0f);
			}
			hlog::write(LOG_TAG, "Disconnected.");
		}
		server->stop();
		delete client;
	}
	delete server;
	sakit::destroy();
	system("pause");
	return 0;
}
