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
	void onBound(sakit::Server* server)
	{
		hlog::writef(LOG_TAG, "- server bound to '%s'", server->getFullHost().c_str());
	}

	void onBindFailed(sakit::Server* server, sakit::Ip host, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- server bind failed to '%s:%d'", host.getAddress().c_str(), port);
	}

	void onUnbound(sakit::Server* server, sakit::Ip host, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- server unbound from '%s:%d'", host.getAddress().c_str(), port);
	}

	void onUnbindFailed(sakit::Server* server)
	{
		hlog::errorf(LOG_TAG, "- server unbind failed from '%s'", server->getFullHost().c_str());
	}

	void onStopped(sakit::Server* server)
	{
		hlog::write(LOG_TAG, "- server stopped");
	}

	void onRunFailed(sakit::Server* server)
	{
		hlog::error(LOG_TAG, "- server running error");
	}

	void onAccepted(sakit::Server* server, sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- server '%s' accepted connection '%s'",
			server->getFullHost().c_str(), socket->getFullHost().c_str());
		socket->receive(12); // receive 12 bytes max
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

class BasicDelegate : public sakit::SocketDelegate
{
public:
	BasicDelegate(chstr name) : sakit::SocketDelegate()
	{
		this->name = name;
	}

	void onSent(sakit::Socket* socket, int bytes)
	{
		hlog::writef(LOG_TAG, "- %s sent %d bytes ", this->name.c_str(), bytes);
	}

	void onSendFinished(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s send finished", this->name.c_str());
	}

	void onSendFailed(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s send failed", this->name.c_str());
	}

	void onReceived(sakit::Socket* socket, hsbase* stream)
	{
		hstr data;
		hstr hex = "0x";
		char c;
		while (!stream->eof())
		{
			stream->read_raw(&c, 1);
			data += c;
			hex += hsprintf("%02X", c);
		}
		hlog::writef(LOG_TAG, "- %s received %d bytes:", this->name.c_str(), stream->size());
		hlog::write(LOG_TAG, data);
		hlog::write(LOG_TAG, hex);
	}

	void onReceiveFinished(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s receive finished", this->name.c_str());
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

	void onReceiveFailed(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s receive failed", this->name.c_str());
		socket->disconnect();
	}

protected:
	hstr name;

};

class ClientDelegate : public BasicDelegate
{
public:
	ClientDelegate() : BasicDelegate("CLIENT")
	{
	}

	void onReceiveFinished(sakit::Socket* socket)
	{
		BasicDelegate::onReceiveFinished(socket);
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

	void onReceiveFailed(sakit::Socket* socket)
	{
		BasicDelegate::onReceiveFailed(socket);
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

} clientDelegate;

class AcceptedDelegate : public BasicDelegate
{
public:
	AcceptedDelegate() : BasicDelegate("ACCEPTED")
	{
	}

	void onSendFinished(sakit::Socket* socket)
	{
		BasicDelegate::onSendFinished(socket);
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

	void onSendFailed(sakit::Socket* socket)
	{
		BasicDelegate::onSendFailed(socket);
		// TODOsock - change this and add isReceiving() and isSending() instead on main thread
		socket->disconnect();
	}

} acceptedDelegate;

void _testAsyncServer()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: async server, blocking client");
	hlog::debug(LOG_TAG, "");
	sakit::Server* server = new sakit::TcpServer(&serverDelegate, &acceptedDelegate);
	if (server->bindAsync(sakit::Ip::Localhost, TEST_PORT))
	{
		while (server->isBinding())
		{
			sakit::update(0.0f);
			hthread::sleep(100.0f);
		}
		if (server->isBound())
		{
			if (server->startAsync())
			{
				sakit::Socket* client = new sakit::TcpSocket(&clientDelegate);
				if (client->connect(sakit::Ip::Localhost, TEST_PORT))
				{
					hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
					hstream stream;
					char data[12] = "Hi there.\0g";
					stream.write_raw(data, 12);
					client->send(&stream);
					client->receive(6); // receive 16 bytes max
					while (client->isConnected())
					{
						sakit::update(0.0f);
						hthread::sleep(100.0f);
					}
					hlog::write(LOG_TAG, "Disconnected.");
				}
				server->stopAsync();
				while (server->isRunning())
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				}
				delete client;
			}
			if (server->unbindAsync())
			{
				while (server->isBinding())
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				}
				if (!server->isBound())
				{
					hlog::write(LOG_TAG, "Unbound");
				}
			}
		}
	}
	delete server;
}

void _testAsyncClient()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: blocking server, async client");
	hlog::debug(LOG_TAG, "");
	/*
	sakit::Server* server = new sakit::TcpServer(&serverDelegate, &acceptedDelegate);
	if (server->bind(sakit::Ip::Localhost, TEST_PORT))
	{
		hlog::write(LOG_TAG, "Bound to " + server->getFullHost());
		server->startAsync();
		sakit::Socket* client = new sakit::TcpSocket(&clientDelegate);
		if (client->connect(sakit::Ip::Localhost, TEST_PORT))
		{
			hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
			hstream stream;
			char data[12] = "Hi there.\0g";
			stream.write_raw(data, 12);
			client->send(&stream);
			client->receive(6); // receive 16 bytes max
			while (client->isConnected())
			{
				sakit::update(0.0f);
				hthread::sleep(100.0f);
			}
			hlog::write(LOG_TAG, "Disconnected.");
		}
		server->stopAsync();
		delete client;
	}
	delete server;
	*/
}

int main(int argc, char **argv)
{
	sakit::init();
	hlog::setLevelDebug(true);
	_testAsyncServer();
	_testAsyncClient();
	hlog::warn(LOG_TAG, "Notice how \\0 characters behave properly when sent over network, but are still problematic in strings.");
	sakit::destroy();
	system("pause");
	return 0;
}
