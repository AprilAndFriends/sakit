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
		hlog::writef(LOG_TAG, "- SERVER bound to '%s'", server->getFullHost().c_str());
	}

	void onBindFailed(sakit::Server* server, sakit::Ip host, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER bind failed to '%s:%d'", host.getAddress().c_str(), port);
	}

	void onUnbound(sakit::Server* server, sakit::Ip host, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER unbound from '%s:%d'", host.getAddress().c_str(), port);
	}

	void onUnbindFailed(sakit::Server* server)
	{
		hlog::errorf(LOG_TAG, "- SERVER unbind failed from '%s'", server->getFullHost().c_str());
	}

	void onStopped(sakit::Server* server)
	{
		hlog::write(LOG_TAG, "- SERVER stopped");
	}

	void onRunFailed(sakit::Server* server)
	{
		hlog::error(LOG_TAG, "- SERVER running error");
	}

	void onAccepted(sakit::Server* server, sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- SERVER '%s' accepted connection '%s'",
			server->getFullHost().c_str(), socket->getFullHost().c_str());
		hsbase* newStream = socket->receive(12); // receive 12 bytes max
		hlog::write(LOG_TAG, "ACCEPTED received: " + hstr(newStream->size()));
		delete newStream;
		hstream stream;
		stream.write("Hello.");
		int sent = socket->send(&stream);
		hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
	}

} serverDelegate;

class SocketDelegate : public sakit::SocketDelegate
{
public:
	SocketDelegate(chstr name) : sakit::SocketDelegate()
	{
		this->name = name;
	}

	void onConnected(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s connected to '%s'", this->name.c_str(), socket->getFullHost().c_str());
	}

	void onDisconnected(sakit::Socket* socket, sakit::Ip host, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- %s disconnected from '%s:%d'", this->name.c_str(), host.getAddress().c_str(), port);
	}

	void onConnectFailed(sakit::Socket* socket, sakit::Ip host, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- %s connect filed to '%s:%d'", this->name.c_str(), host.getAddress().c_str(), port);
	}

	void onDisconnectFailed(sakit::Socket* socket)
	{
		hlog::errorf(LOG_TAG, "- %s disconnect failed from '%s'", this->name.c_str(), socket->getFullHost().c_str());
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
	}

	void onReceiveFailed(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s receive failed", this->name.c_str());
	}

protected:
	hstr name;

};

SocketDelegate clientDelegate("CLIENT");
SocketDelegate acceptedDelegate("ACCEPTED");

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
					int sent = client->send(&stream);
					hlog::write(LOG_TAG, "Client sent: " + hstr(sent));
					while (server->getSockets().size() == 0)
					{
						sakit::update(0.0f);
						hthread::sleep(100.0f);
					}
					hsbase* newStream = client->receive(6); // receive 6 bytes max
					hlog::write(LOG_TAG, "Client received: " + hstr(newStream->size()));
					delete newStream;
					hlog::write(LOG_TAG, "Disconnected.");
					server->destroy(server->getSockets()[0]);
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
				while (server->isUnbinding())
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
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
	sakit::Server* server = new sakit::TcpServer(&serverDelegate, &acceptedDelegate);
	if (server->bind(sakit::Ip::Localhost, TEST_PORT))
	{
		hlog::writef(LOG_TAG, "Server bound to '%s'", server->getFullHost().c_str());
		sakit::Socket* client = new sakit::TcpSocket(&clientDelegate);
		if (client->connectAsync(sakit::Ip::Localhost, TEST_PORT))
		{
			sakit::Socket* accepted = NULL;
			while (accepted == NULL)
			{
				sakit::update(0.0f);
				accepted = server->accept(0.1f);
				hthread::sleep(100.0f);
			}
			do
			{
				sakit::update(0.0f);
				hthread::sleep(100.0f);
			} while (client->isConnecting());
			if (client->isConnected()) // should be true after accepting a connection
			{
				hstream stream;
				char data[12] = "Hi there.\0g";
				stream.write_raw(data, 12);
				client->sendAsync(&stream);
				do
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				} while (client->isSending());
				// accepted handling
				hsbase* newStream = accepted->receive(12); // receive 12 bytes max
				hlog::write(LOG_TAG, "ACCEPTED received: " + hstr(newStream->size()));
				delete newStream;
				stream.clear();
				stream.write("Hello.");
				int sent = accepted->send(&stream);
				hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
				// back to client
				client->receiveAsync(6); // receive 6 bytes max
				do
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				} while (client->isReceiving());
				client->disconnectAsync();
				do
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				} while (client->isDisconnecting());
			}
			server->destroy(accepted);
		}
		delete client;
		if (server->unbind())
		{
			hlog::write(LOG_TAG, "Server unbound.");
		}
	}
	delete server;
}

int main(int argc, char **argv)
{
	hlog::setLevelDebug(true);
	sakit::init();
	_testAsyncServer();
	_testAsyncClient();
	hlog::warn(LOG_TAG, "Notice how \\0 characters behave properly when sent over network, but are still problematic in strings.");
	sakit::destroy();
	system("pause");
	return 0;
}
