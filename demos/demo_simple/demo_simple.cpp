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
#include <sakit/Socket.h>
#include <sakit/SocketDelegate.h>
#include <sakit/TcpServer.h>
#include <sakit/TcpServerDelegate.h>
#include <sakit/TcpSocket.h>
#include <sakit/UdpServer.h>
#include <sakit/UdpServerDelegate.h>
#include <sakit/UdpSocket.h>
#include <sakit/HttpResponse.h>
#include <sakit/HttpSocket.h>
#include <sakit/HttpSocketDelegate.h>

#define TEST_PORT 53334

class TcpServerDelegate : public sakit::TcpServerDelegate
{
	void onBound(sakit::Server* server)
	{
		hlog::writef(LOG_TAG, "- SERVER bound to '%s'", server->getFullHost().c_str());
	}

	void onBindFailed(sakit::Server* server, sakit::Host host, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER bind failed to '%s:%d'", host.toString().c_str(), port);
	}

	void onUnbound(sakit::Server* server, sakit::Host host, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER unbound from '%s:%d'", host.toString().c_str(), port);
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

	void onAccepted(sakit::TcpServer* server, sakit::TcpSocket* socket)
	{
		hlog::writef(LOG_TAG, "- SERVER '%s' accepted connection '%s'", server->getFullHost().c_str(), socket->getFullHost().c_str());
		hstream stream;
		socket->receive(&stream, 12); // receive 12 bytes max
		hlog::write(LOG_TAG, "ACCEPTED received: " + hstr((int) stream.size()));
		int sent = socket->send("Hello.");
		hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
	}

} tcpServerDelegate;

class UdpServerDelegate : public sakit::UdpServerDelegate
{
	void onBound(sakit::Server* server)
	{
		hlog::writef(LOG_TAG, "- SERVER bound to '%s'", server->getFullHost().c_str());
	}

	void onBindFailed(sakit::Server* server, sakit::Host host, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER bind failed to '%s:%d'", host.toString().c_str(), port);
	}

	void onUnbound(sakit::Server* server, sakit::Host host, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER unbound from '%s:%d'", host.toString().c_str(), port);
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

	void onReceived(sakit::UdpServer* server, sakit::UdpSocket* socket, hstream* stream)
	{
		hlog::writef(LOG_TAG, "- SERVER '%s' accepted connection '%s'", server->getFullHost().c_str(), socket->getFullHost().c_str());
		hlog::write(LOG_TAG, "ACCEPTED received: " + hstr((int) stream->size()));
		int sent = socket->send("Hello.");
		hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
	}

} udpServerDelegate;

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

	void onDisconnected(sakit::Socket* socket, sakit::Host host, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- %s disconnected from '%s:%d'", this->name.c_str(), host.toString().c_str(), port);
	}

	void onConnectFailed(sakit::Socket* socket, sakit::Host host, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- %s connect filed to '%s:%d'", this->name.c_str(), host.toString().c_str(), port);
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

	void onReceived(sakit::Socket* socket, hstream* stream)
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
SocketDelegate receivedDelegate("RECEIVED");

class HttpSocketDelegate : public sakit::HttpSocketDelegate
{
public:
	HttpSocketDelegate() : sakit::HttpSocketDelegate()
	{
	}

	void onExecuteCompleted(sakit::HttpSocket* socket, sakit::HttpResponse* response, sakit::Url url)
	{
		hlog::debugf(LOG_TAG, "- received %d bytes from %s", response->Raw.size(), url.getHost().c_str());
		hlog::write(LOG_TAG, response->Raw.read(1500) + "\n...");
	}

	void onExecuteFailed(sakit::HttpSocket* socket, sakit::HttpResponse* response, sakit::Url url)
	{
	}

} httpSocketDelegate;

void _testAsyncTcpServer()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: async TCP server, blocking TCP client");
	hlog::debug(LOG_TAG, "");
	sakit::TcpServer* server = new sakit::TcpServer(&tcpServerDelegate, &acceptedDelegate);
	if (server->bindAsync(sakit::Host::Localhost, TEST_PORT))
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
				sakit::TcpSocket* client = new sakit::TcpSocket(&clientDelegate);
				if (client->connect(sakit::Host::Localhost, TEST_PORT))
				{
					hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
					hstream stream;
					char data[12] = "Hi there.\0g";
					stream.write_raw(data, 12);
					stream.rewind();
					int sent = client->send(&stream);
					hlog::write(LOG_TAG, "Client sent: " + hstr(sent));
					while (server->getSockets().size() == 0)
					{
						sakit::update(0.0f);
						hthread::sleep(100.0f);
					}
					stream.clear();
					client->receive(&stream, 20); // receive 20 bytes max
					hlog::write(LOG_TAG, "Client received: " + hstr((int) stream.size()));
					hlog::write(LOG_TAG, "Disconnected.");
					server->getSockets()[0]->disconnect();
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

void _testAsyncTcpClient()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: blocking TCP server, async TCP client");
	hlog::debug(LOG_TAG, "");
	sakit::TcpServer* server = new sakit::TcpServer(&tcpServerDelegate, &acceptedDelegate);
	if (server->bind(sakit::Host::Localhost, TEST_PORT))
	{
		hlog::writef(LOG_TAG, "Server bound to '%s'", server->getFullHost().c_str());
		sakit::TcpSocket* client = new sakit::TcpSocket(&clientDelegate);
		if (client->connectAsync(sakit::Host::Localhost, TEST_PORT))
		{
			sakit::TcpSocket* accepted = NULL;
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
				stream.rewind();
				client->sendAsync(&stream);
				do
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				} while (client->isSending());
				// accepted handling
				stream.clear();
				accepted->receive(&stream, 12); // receive 12 bytes max
				hlog::write(LOG_TAG, "ACCEPTED received: " + hstr((int) stream.size()));
				int sent = accepted->send("Hello.");
				hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
				// back to client
				client->startReceiveAsync(); // start receiving
				for_iter (i, 0, 10)
				{
					sakit::update(0.0f);
					hthread::sleep(100.0f);
				}
				client->stopReceiveAsync();
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
			accepted->disconnect();
		}
		delete client;
		if (server->unbind())
		{
			hlog::write(LOG_TAG, "Server unbound.");
		}
	}
	delete server;
}

void _testAsyncUdpServer()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: async UDP server, blocking UDP client");
	hlog::debug(LOG_TAG, "");
	sakit::UdpServer* server = new sakit::UdpServer(&udpServerDelegate, &receivedDelegate);
	if (server->bindAsync(sakit::Host::Localhost, TEST_PORT))
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
				sakit::UdpSocket* client = new sakit::UdpSocket(&clientDelegate);
				if (client->setDestination(sakit::Host::Localhost, TEST_PORT))
				{
					hlog::write(LOG_TAG, "Connected to " + client->getFullHost());
					hstream stream;
					char data[12] = "Hi there.\0g";
					stream.write_raw(data, 12);
					stream.rewind();
					int sent = client->send(&stream);
					hlog::write(LOG_TAG, "Client sent: " + hstr(sent));
					while (server->getSockets().size() == 0)
					{
						sakit::update(0.0f);
						hthread::sleep(100.0f);
					}
					stream.clear();
					client->receive(&stream); // receive all there is
					hlog::write(LOG_TAG, "Client received: " + hstr((int) stream.size()));
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

void _testAsyncUdpClient()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: blocking TCP server, async TCP client");
	hlog::debug(LOG_TAG, "");
	sakit::UdpServer* server = new sakit::UdpServer(&udpServerDelegate, &receivedDelegate);
	if (server->bind(sakit::Host::Localhost, TEST_PORT))
	{
		hlog::writef(LOG_TAG, "Server bound to '%s'", server->getFullHost().c_str());
		sakit::UdpSocket* client = new sakit::UdpSocket(&clientDelegate);
		if (client->setDestination(sakit::Host::Localhost, TEST_PORT)) // there is no async destination setting, it's not needed
		{
			hstream stream;
			char data[12] = "Hi there.\0g";
			stream.write_raw(data, 12);
			stream.rewind();
			client->sendAsync(&stream);
			do
			{
				sakit::update(0.0f);
				hthread::sleep(100.0f);
			} while (client->isSending());
			// received handling
			stream.clear();
			sakit::UdpSocket* received = server->receive(&stream);
			hlog::write(LOG_TAG, "RECEIVED received: " + hstr((int) stream.size()));
			int sent = received->send("Hello.");
			hlog::write(LOG_TAG, "RECEIVED sent: " + hstr(sent));
			// back to client
			client->startReceiveAsync(); // start receiving
			for_iter (i, 0, 10)
			{
				sakit::update(0.0f);
				hthread::sleep(100.0f);
			}
			client->stopReceiveAsync();
			do
			{
				sakit::update(0.0f);
				hthread::sleep(100.0f);
			} while (client->isReceiving());
			client->clearDestination(); // there is no async destination clearing, it's not needed
			received->clearDestination();
		}
		delete client;
		if (server->unbind())
		{
			hlog::write(LOG_TAG, "Server unbound.");
		}
	}
	delete server;
}

void _testHttpSocket()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: blocking HTTP client");
	hlog::debug(LOG_TAG, "");
	sakit::HttpSocket* client = new sakit::HttpSocket(&httpSocketDelegate);
	sakit::HttpResponse response;
	sakit::Url url("http://en.wikipedia.org/wiki/C++#Polymorphism");
	hlog::debug(LOG_TAG, "URL: " + url.toString());
	hmap<hstr, hstr> headers;
	headers["User-Agent"] = "SAKit Demo Simple";
	if (client->executeGet(&response, url, headers))
	{
		hlog::debugf(LOG_TAG, "- received %d bytes from %s", response.Raw.size(), url.getHost().c_str());
		hlog::write(LOG_TAG, response.Raw.read(1500) + "\n...");
	}
	else
	{
		hlog::error(LOG_TAG, "Failed to call " SAKIT_HTTP_REQUEST_GET " on: " + url.getHost());
	}
	delete client;
}

void _testAsyncHttpSocket()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: async HTTP client");
	hlog::debug(LOG_TAG, "");
	sakit::HttpSocket* client = new sakit::HttpSocket(&httpSocketDelegate);
	sakit::HttpResponse response;
	sakit::Url url("http://en.wikipedia.org/wiki/C++#Polymorphism");
	hlog::debug(LOG_TAG, "URL: " + url.toString());
	hmap<hstr, hstr> headers;
	headers["User-Agent"] = "SAKit Demo Simple";
	if (client->executeGetAsync(url, headers))
	{
		do
		{
			sakit::update(0.0f);
			hthread::sleep(100.0f);
		} while (client->isExecuting());
	}
	else
	{
		hlog::error(LOG_TAG, "Failed to call " SAKIT_HTTP_REQUEST_GET " on: " + url.getHost());
	}
	delete client;
}

#ifndef _WINRT
int main(int argc, char **argv)
#else
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
#endif
{
	hlog::setLevelDebug(true); // for the nice colors
	sakit::init();
#ifndef _WINRT
	// TCP tests
	_testAsyncTcpServer();
	_testAsyncTcpClient();
	// UDP tests
	_testAsyncUdpServer();
	_testAsyncUdpClient();
#endif
	hlog::warn(LOG_TAG, "Notice how \\0 characters behave properly when sent over network, but are still problematic in strings.");
	// HTTP tests
	sakit::setRetryTimeout(0.01f);
	sakit::setRetryAttempts(1000); // makes for a 10 second timeout
	_testHttpSocket();
	_testAsyncHttpSocket();
	// done
	hlog::debug(LOG_TAG, "Done.");
	sakit::destroy();
#if defined(_WIN32) && !defined(_WINRT)
	system("pause");
#endif
	return 0;
}
