/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#define LOG_TAG "demo_simple"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include <sakit/sakit.h>
#include <sakit/Socket.h>
#include <sakit/TcpServer.h>
#include <sakit/TcpServerDelegate.h>
#include <sakit/TcpSocket.h>
#include <sakit/TcpSocketDelegate.h>
#include <sakit/UdpServer.h>
#include <sakit/UdpServerDelegate.h>
#include <sakit/UdpSocket.h>
#include <sakit/UdpSocketDelegate.h>
#include <sakit/HttpResponse.h>
#include <sakit/HttpSocket.h>
#include <sakit/HttpSocketDelegate.h>

#define TCP_PORT_SYNC_SERVER 50000
#define TCP_PORT_ASYNC_SERVER 50001
#define UDP_PORT_SYNC_SERVER 50100
#define UDP_PORT_ASYNC_SERVER 50101
#define UDP_PORT_SYNC_SERVER_ANSWER 50110
#define UDP_PORT_ASYNC_SERVER_ANSWER 50111
#define UDP_PORT_SYNC_CLIENT 50200
#define UDP_PORT_ASYNC_CLIENT 50201
#define UDP_PORT_MULTICAST_CLIENT_1 50300
#define UDP_PORT_MULTICAST_CLIENT_2 50301
#define UDP_PORT_BROADCAST 51000
#define UDP_MULTICAST_HOST_ADDRESS "192.168.1.109" // this needs changing depending on the machine
#define UDP_MULTICAST_ADDRESS "226.2.3.4"

void _printReceived(hstream* stream)
{
	hstr string;
	hstr hex = "0x";
	char c;
	while (!stream->eof())
	{
		stream->read_raw(&c, 1);
		string += c;
		hex += hsprintf("%02X", c);
	}
	hlog::write(LOG_TAG, "-> " + string);
	hlog::write(LOG_TAG, "-> " + hex);
}

class TcpSocketDelegate : public sakit::TcpSocketDelegate
{
public:
	TcpSocketDelegate(chstr name) : sakit::TcpSocketDelegate()
	{
		this->name = name;
	}

	void onConnected(sakit::Connector* connector, sakit::Host remoteHost, unsigned short remotePort)
	{
		hlog::writef(LOG_TAG, "- %s connected to '%s:%d'", this->name.c_str(), remoteHost.toString().c_str(), remotePort);
	}

	void onDisconnected(sakit::Connector* connector, sakit::Host remoteHost, unsigned short remotePort)
	{
		hlog::writef(LOG_TAG, "- %s disconnected from '%s:%d'", this->name.c_str(), remoteHost.toString().c_str(), remotePort);
	}

	void onConnectFailed(sakit::Connector* connector, sakit::Host remoteHost, unsigned short remotePort)
	{
		hlog::errorf(LOG_TAG, "- %s connect failed to '%s:%d'", this->name.c_str(), remoteHost.toString().c_str(), remotePort);
	}

	void onDisconnectFailed(sakit::Connector* connector, sakit::Host remoteHost, unsigned short remotePort)
	{
		hlog::errorf(LOG_TAG, "- %s disconnect failed from '%s:%d'", this->name.c_str(), remoteHost.toString().c_str(), remotePort);
	}

	void onSent(sakit::Socket* socket, int bytes)
	{
		hlog::writef(LOG_TAG, "- %s sent: %d", this->name.c_str(), bytes);
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
		hlog::writef(LOG_TAG, "- %s received: %d", this->name.c_str(), stream->size());
		_printReceived(stream);
	}

	void onReceiveFinished(sakit::TcpSocket* socket)
	{
		hlog::writef(LOG_TAG, "- %s receive finished", this->name.c_str());
	}

	void onReceiveFailed(sakit::TcpSocket* socket)
	{
		hlog::writef(LOG_TAG, "- %s receive failed", this->name.c_str());
	}

protected:
	hstr name;

};

class UdpSocketDelegate : public sakit::UdpSocketDelegate
{
public:
	UdpSocketDelegate(chstr name) : sakit::UdpSocketDelegate()
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

	void onReceived(sakit::UdpSocket* socket, sakit::Host remoteHost, unsigned short port, hstream* stream)
	{
		hlog::writef(LOG_TAG, "- %s received %d bytes (from '%s:%d'): %d", this->name.c_str(), stream->size(), remoteHost.toString().c_str(), port, stream->size());
		_printReceived(stream);
	}

	void onReceiveFinished(sakit::Socket* socket)
	{
		hlog::writef(LOG_TAG, "- %s receive finished", this->name.c_str());
	}

	void onBroadcastFinished(sakit::UdpSocket* socket)
	{
		hlog::writef(LOG_TAG, "- %s broadcast finished", this->name.c_str());
	}

	void onBroadcastFailed(sakit::UdpSocket* socket)
	{
		hlog::writef(LOG_TAG, "- %s broadcast failed", this->name.c_str());
	}

protected:
	hstr name;

};

TcpSocketDelegate tcpClientDelegate("CLIENT");
TcpSocketDelegate tcpAcceptedDelegate("ACCEPTED");
UdpSocketDelegate udpClientDelegate("CLIENT");

class TcpServerDelegate : public sakit::TcpServerDelegate
{
	void onBound(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER bound to '%s:%d'", localHost.toString().c_str(), port);
	}

	void onBindFailed(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER bind failed to '%s:%d'", localHost.toString().c_str(), port);
	}

	void onUnbound(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER unbound from '%s:%d'", localHost.toString().c_str(), port);
	}

	void onUnbindFailed(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER unbind failed from '%s:%d'", localHost.toString().c_str(), port);
	}

	void onStopped(sakit::Server* server)
	{
		hlog::write(LOG_TAG, "- SERVER stopped");
	}

	void onStartFailed(sakit::Server* server)
	{
		hlog::error(LOG_TAG, "- SERVER start failed");
	}

	void onAccepted(sakit::TcpServer* server, sakit::TcpSocket* socket)
	{
		hlog::writef(LOG_TAG, "- SERVER '%s:%d' accepted connection '%s:%d'", server->getLocalHost().toString().c_str(),
			server->getLocalPort(), socket->getRemoteHost().toString().c_str(), socket->getRemotePort());
		hstream stream;
		socket->receive(&stream, 12); // receive 12 bytes max
		stream.rewind();
		hlog::write(LOG_TAG, "ACCEPTED received: " + hstr((int)stream.size()));
		_printReceived(&stream);
		int sent = socket->send("Hello.");
		hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
	}

} tcpServerDelegate;

class UdpServerDelegate : public sakit::UdpServerDelegate
{
	void onBound(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER bound to '%s:%d'", localHost.toString().c_str(), port);
	}

	void onBindFailed(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER bind failed to '%s:%d'", localHost.toString().c_str(), port);
	}

	void onUnbound(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::writef(LOG_TAG, "- SERVER unbound from '%s:%d'", localHost.toString().c_str(), port);
	}

	void onUnbindFailed(sakit::Binder* binder, sakit::Host localHost, unsigned short port)
	{
		hlog::errorf(LOG_TAG, "- SERVER unbind failed from '%s:%d'", localHost.toString().c_str(), port);
	}

	void onStopped(sakit::Server* server)
	{
		hlog::write(LOG_TAG, "- SERVER stopped");
	}

	void onStartFailed(sakit::Server* server)
	{
		hlog::error(LOG_TAG, "- SERVER running error");
	}

	void onReceived(sakit::UdpServer* server, sakit::Host remoteHost, unsigned short remotePort, hstream* stream)
	{
		hlog::writef(LOG_TAG, "- SERVER '%s:%d' received data (from '%s:%d'): %d", server->getLocalHost().toString().c_str(), server->getLocalPort(), remoteHost.toString().c_str(), remotePort, stream->size());
		_printReceived(stream);
		sakit::UdpSocket* udpSocket = new sakit::UdpSocket(&udpClientDelegate);
		if (udpSocket->bind(sakit::Host::Localhost, UDP_PORT_ASYNC_SERVER_ANSWER))
		{
			hlog::writef(LOG_TAG, "UDP-Socket bound to '%s:%d'", udpSocket->getLocalHost().toString().c_str(), udpSocket->getLocalPort());
			if (udpSocket->setDestination(remoteHost, remotePort))
			{
				hlog::writef(LOG_TAG, "UDP-Socket connected to '%s:%d'", udpSocket->getRemoteHost().toString().c_str(), udpSocket->getRemotePort());
				int sent = udpSocket->send("Hello.");
				hlog::write(LOG_TAG, "UDP-SOCKET sent: " + hstr(sent));
			}
			else
			{
				hlog::error(LOG_TAG, "Could not set destination!");
			}
		}
		else
		{
			hlog::error(LOG_TAG, "Could not bind UDP socket!");
		}
		delete udpSocket;
	}

} udpServerDelegate;

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
	sakit::TcpServer* server = new sakit::TcpServer(&tcpServerDelegate, &tcpAcceptedDelegate);
	if (server->bindAsync(sakit::Host::Localhost, TCP_PORT_ASYNC_SERVER))
	{
		while (server->isBinding())
		{
			sakit::update();
			hthread::sleep(100.0f);
		}
		if (server->isBound())
		{
			if (server->startAsync())
			{
				sakit::TcpSocket* client = new sakit::TcpSocket(&tcpClientDelegate);
				if (client->connect(sakit::Host::Localhost, TCP_PORT_ASYNC_SERVER))
				{
					hlog::writef(LOG_TAG, "Connected to '%s:%d'", client->getRemoteHost().toString().c_str(), client->getRemotePort());
					hstream stream;
					char data[12] = "Hi there.\0g";
					stream.writeRaw(data, 12);
					stream.rewind();
					int sent = client->send(&stream);
					hlog::write(LOG_TAG, "Client sent: " + hstr(sent));
					while (server->getSockets().size() == 0)
					{
						sakit::update();
						hthread::sleep(100.0f);
					}
					stream.clear();
					client->receive(&stream, 20); // receive 20 bytes max
					stream.rewind();
					hlog::write(LOG_TAG, "Client received: " + hstr((int)stream.size()));
					_printReceived(&stream);
					hlog::write(LOG_TAG, "Disconnected.");
					server->getSockets()[0]->disconnect();
				}
				server->stopAsync();
				while (server->isRunning())
				{
					sakit::update();
					hthread::sleep(100.0f);
				}
				delete client;
			}
			if (server->unbindAsync())
			{
				while (server->isUnbinding())
				{
					sakit::update();
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
	sakit::TcpServer* server = new sakit::TcpServer(&tcpServerDelegate, &tcpAcceptedDelegate);
	server->setTimeout(1.0f);
	if (server->bind(sakit::Host::Localhost, TCP_PORT_SYNC_SERVER))
	{
		hlog::writef(LOG_TAG, "Server bound to '%s:%d'", server->getLocalHost().toString().c_str(), server->getLocalPort());
		sakit::TcpSocket* client = new sakit::TcpSocket(&tcpClientDelegate);
		if (client->connectAsync(sakit::Host::Localhost, TCP_PORT_SYNC_SERVER))
		{
			sakit::TcpSocket* accepted = NULL;
			while (accepted == NULL)
			{
				sakit::update();
				accepted = server->accept();
				hthread::sleep(100.0f);
			}
			do
			{
				sakit::update();
				hthread::sleep(100.0f);
			} while (client->isConnecting());
			if (client->isConnected()) // should be true after accepting a connection
			{
				hstream stream;
				char data[12] = "Hi there.\0g";
				stream.writeRaw(data, 12);
				stream.rewind();
				client->sendAsync(&stream);
				do
				{
					sakit::update();
					hthread::sleep(100.0f);
				} while (client->isSending());
				// accepted handling
				stream.clear();
				accepted->receive(&stream, 12); // receive 12 bytes max
				stream.rewind();
				hlog::write(LOG_TAG, "ACCEPTED received: " + hstr((int)stream.size()));
				_printReceived(&stream);
				int sent = accepted->send("Hello.");
				hlog::write(LOG_TAG, "ACCEPTED sent: " + hstr(sent));
				// back to client
				client->startReceiveAsync(); // start receiving
				for_iter (i, 0, 10)
				{
					sakit::update();
					hthread::sleep(100.0f);
				}
				client->stopReceiveAsync();
				do
				{
					sakit::update();
					hthread::sleep(100.0f);
				} while (client->isReceiving());
				client->disconnectAsync();
				do
				{
					sakit::update();
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
	sakit::UdpServer* server = new sakit::UdpServer(&udpServerDelegate);
	if (server->bindAsync(sakit::Host::Localhost, UDP_PORT_ASYNC_SERVER))
	{
		while (server->isBinding())
		{
			sakit::update();
			hthread::sleep(100.0f);
		}
		if (server->isBound())
		{
			if (server->startAsync())
			{
				sakit::UdpSocket* client = new sakit::UdpSocket(&udpClientDelegate);
				if (client->bind(sakit::Host::Localhost, UDP_PORT_SYNC_CLIENT))
				{
					hlog::writef(LOG_TAG, "Bound to '%s:%d'", client->getLocalHost().toString().c_str(), client->getLocalPort());
					if (client->setDestination(sakit::Host::Localhost, UDP_PORT_ASYNC_SERVER))
					{
						hlog::writef(LOG_TAG, "Connected to '%s:%d'", client->getRemoteHost().toString().c_str(), client->getRemotePort());
						hstream stream;
						char data[12] = "Hi there.\0g";
						stream.writeRaw(data, 12);
						stream.rewind();
						int sent = client->send(&stream);
						hlog::write(LOG_TAG, "Client sent: " + hstr(sent));
						for_iter (i, 0, 10)
						{
							sakit::update();
							hthread::sleep(100.0f);
						}
						stream.clear();
						sakit::Host remoteHost;
						unsigned short remotePort = 0;
						client->receive(&stream, remoteHost, remotePort); // receive all there is
						stream.rewind();
						hlog::writef(LOG_TAG, "Client received (from '%s:%d'): %d", remoteHost.toString().c_str(), remotePort, stream.size());
						_printReceived(&stream);
						hlog::write(LOG_TAG, "Disconnected.");
					}
				}
				server->stopAsync();
				while (server->isRunning())
				{
					sakit::update();
					hthread::sleep(100.0f);
				}
				delete client;
			}
			if (server->unbindAsync())
			{
				while (server->isUnbinding())
				{
					sakit::update();
					hthread::sleep(100.0f);
				}
			}
		}
	}
	delete server;
	//*/
}

void _testAsyncUdpClient()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: blocking UDP server, async UDP client");
	hlog::debug(LOG_TAG, "");
	sakit::UdpServer* server = new sakit::UdpServer(&udpServerDelegate);
	if (server->bind(sakit::Host::Localhost, UDP_PORT_SYNC_SERVER))
	{
		hlog::writef(LOG_TAG, "Server bound to '%s:%d'", server->getLocalHost().toString().c_str(), server->getLocalPort());
		sakit::UdpSocket* client = new sakit::UdpSocket(&udpClientDelegate);
		if (client->bindAsync(sakit::Host::Localhost, UDP_PORT_ASYNC_CLIENT))
		{
			do
			{
				sakit::update();
				hthread::sleep(100.0f);
			} while (client->isBinding());
			hlog::writef(LOG_TAG, "CLIENT bound to '%s:%d'", client->getLocalHost().toString().c_str(), client->getLocalPort());
			if (client->setDestination(sakit::Host::Localhost, UDP_PORT_SYNC_SERVER)) // there is no async destination setting, it's not needed
			{
				hstream stream;
				char data[12] = "Hi there.\0g";
				stream.writeRaw(data, 12);
				stream.rewind();
				client->sendAsync(&stream);
				do
				{
					sakit::update();
					hthread::sleep(100.0f);
				} while (client->isSending());
				// received handling
				stream.clear();
				sakit::Host remoteHost;
				unsigned short remotePort = 0;
				if (server->receive(&stream, remoteHost, remotePort))
				{
					stream.rewind();
					hlog::writef(LOG_TAG, "RECEIVED received (from '%s:%d'): %d", remoteHost.toString().c_str(), remotePort, stream.size());
					_printReceived(&stream);
					sakit::UdpSocket* received = new sakit::UdpSocket(&udpClientDelegate);
					if (received->bind(sakit::Host::Localhost, UDP_PORT_SYNC_SERVER_ANSWER))
					{
						hlog::writef(LOG_TAG, "RECEIVED bound to '%s:%d'", received->getLocalHost().toString().c_str(), received->getLocalPort());
						if (received->setDestination(remoteHost, remotePort))
						{
							int sent = received->send("Hello.");
							hlog::write(LOG_TAG, "RECEIVED sent: " + hstr(sent));
							// back to client
							client->startReceiveAsync(); // start receiving
							for_iter (i, 0, 10)
							{
								sakit::update();
								hthread::sleep(100.0f);
							}
							client->stopReceiveAsync();
							do
							{
								sakit::update();
								hthread::sleep(100.0f);
							} while (client->isReceiving());
						}
						else
						{
							hlog::error(LOG_TAG, "Could not set destination!");
						}
					}
					else
					{
						hlog::error(LOG_TAG, "Could not bind UDP socket!");
					}
					delete received;
				}
				else
				{
					hlog::error(LOG_TAG, "Could not receive!");
				}
			}
		}
		else
		{
			hlog::error(LOG_TAG, "Could not bind UDP socket!");
		}
		delete client;
		if (server->unbind())
		{
			hlog::write(LOG_TAG, "Server unbound.");
		}
	}
	delete server;
}

void _testUdpBroadcast()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: UDP broadcast");
	hlog::debug(LOG_TAG, "");
	hlog::write(LOG_TAG, "Remember to run a server first where messages can be checked!");
	sakit::UdpSocket* socket = new sakit::UdpSocket(&udpClientDelegate);
	socket->bind();
	socket->broadcast(UDP_PORT_BROADCAST, "Hello.");
	delete socket;
}

void _testUdpMulticast()
{
	hlog::debug(LOG_TAG, "");
	hlog::debug(LOG_TAG, "starting test: UDP multicast");
	hlog::debug(LOG_TAG, "");
	int sent = 0;
	int received = 0;
	sakit::UdpSocket* s1 = new sakit::UdpSocket(NULL); // not using any async calls here, no delegate needed
	if (s1->bind(sakit::Host(UDP_MULTICAST_HOST_ADDRESS), UDP_PORT_MULTICAST_CLIENT_1))
	{
		hlog::writef(LOG_TAG, "UDP-1 bound to '%s:%d'", s1->getLocalHost().toString().c_str(), s1->getLocalPort());
		if (s1->joinMulticastGroup(s1->getLocalHost(), sakit::Host(UDP_MULTICAST_ADDRESS)))
		{
			hlog::writef(LOG_TAG, "UDP-1 joined multicast group '%s'", UDP_MULTICAST_ADDRESS);
			sakit::UdpSocket* s2 = new sakit::UdpSocket(NULL); // not using any async calls here, no delegate needed
			if (s2->bind(UDP_PORT_MULTICAST_CLIENT_2))
			{
				hlog::writef(LOG_TAG, "UDP-2 bound to '%s:%d'", s2->getLocalHost().toString().c_str(), s2->getLocalPort());
				if (s2->setDestination(sakit::Host(UDP_MULTICAST_ADDRESS), UDP_PORT_MULTICAST_CLIENT_1))
				{
					hlog::writef(LOG_TAG, "UDP-2 set destination to '%s:%d'", s2->getRemoteHost().toString().c_str(), s2->getRemotePort());
					sent = s2->send("Hello.");
					hlog::writef(LOG_TAG, "UDP-2 sent (to '%s:%d'): %d", s2->getRemoteHost().toString().c_str(), s2->getRemotePort(), sent);
					hstream stream;
					sakit::Host remoteHost;
					unsigned short remotePort = 0;
					s1->receive(&stream, remoteHost, remotePort);
					stream.rewind();
					hlog::writef(LOG_TAG, "UDP-1 received (from '%s:%d'): %d", remoteHost.toString().c_str(), remotePort, stream.size());
					if (stream.size() > 0)
					{
						_printReceived(&stream);
						s1->setDestination(remoteHost, remotePort);
						sent = s1->send("Hi.");
						hlog::writef(LOG_TAG, "UDP-1 sent (to '%s:%d'): %d", remoteHost.toString().c_str(), remotePort, sent);
						stream.clear();
						s2->receive(&stream, remoteHost, remotePort);
						stream.rewind();
						hlog::writef(LOG_TAG, "UDP-2 received (from '%s:%d'): %d", remoteHost.toString().c_str(), remotePort, stream.size());
						if (stream.size() > 0)
						{
							_printReceived(&stream);
						}
					}
				}
				else
				{
					hlog::error(LOG_TAG, "Could not set destination!");
				}
			}
			else
			{
				hlog::error(LOG_TAG, "Could not bind UDP socket!");
			}
			delete s2;
		}
		else
		{
			hlog::error(LOG_TAG, "Could not join multicast group!");
		}
	}
	else
	{
		hlog::error(LOG_TAG, "Could not bind UDP socket!");
	}
	delete s1;
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
//	if (client->executeGet(&response, url, headers))
//	{
//		hlog::debugf(LOG_TAG, "- received %d bytes from %s", response.Raw.size(), url.getHost().c_str());
//		hlog::write(LOG_TAG, response.Raw.read(1500) + "\n...");
//	}
//	else
//	{
//		hlog::error(LOG_TAG, "Failed to call " SAKIT_HTTP_REQUEST_GET " on: " + url.getHost());
//	}
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
//	if (client->executeGetAsync(url, headers))
//	{
//		do
//		{
//			sakit::update();
//			hthread::sleep(100.0f);
//		} while (client->isExecuting());
//	}
//	else
//	{
//		hlog::error(LOG_TAG, "Failed to call " SAKIT_HTTP_REQUEST_GET " on: " + url.getHost());
//	}
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
#ifndef _WINRT // because TCP servers are not supported on WinRT
	// TCP tests
	_testAsyncTcpServer();
	_testAsyncTcpClient();
#endif
	// UDP tests
	_testAsyncUdpServer();
	_testAsyncUdpClient();
	_testUdpBroadcast();
#ifndef _WINRT // because loopbacks are disabled on WinRT, multicast messages will not arrive and render this test basically useless
	_testUdpMulticast();
#endif
	hlog::warn(LOG_TAG, "Notice how \\0 characters behave properly when sent over network, but are still problematic in strings.");
	// HTTP tests
	sakit::setGlobalTimeout(10.0f, 0.01f);
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
