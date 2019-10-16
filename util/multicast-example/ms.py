"""
The SocketServer module does not provide support for multicast. This module
provides a subclass of SocketServer.UDPServer that can listen on multicast 
addresses.

This only supports IPv4
"""

import SocketServer
import sys,socket, struct, platform, threading, signal

discoveryServer = None
discoveryThread = None
cgsyncServer = None
cgsyncThread = None

class ServerDiscoveryServer(SocketServer.UDPServer):
    """Extends UDPServer to join multicast groups and bind
    the local interface properly
    """

    def __init__(self, multicast_address, RequestHandlerClass, listen_interfaces = None):
        #to receive multicast packets, must bind the port, 
        #set bind_and_active to True. 
        #Note: some hosts don't allow bind()'ing to a multicast address, 
        #so bind to INADDR_ANY
        SocketServer.UDPServer.__init__(self, ('', multicast_address[1]),
                                              RequestHandlerClass, True)

        #Note: struct ip_mreq { struct in_addr (multicast addr), struct in_addr 
        #(local interface) }
        if listen_interfaces is None:
            mreq = struct.pack("4sI", socket.inet_aton(multicast_address[0]),
                               socket.INADDR_ANY)
            self.socket.setsockopt(socket.IPPROTO_IP,
                                       socket.IP_ADD_MEMBERSHIP, mreq)
        else:
            for interface in listen_interfaces:
                mreq = socket.inet_aton(
                    multicast_address[0]) + socket.inet_aton(interface)
                self.socket.setsockopt(socket.IPPROTO_IP,
                                       socket.IP_ADD_MEMBERSHIP, mreq)

    def server_close(self):
        #TODO: leave the multicast groups...
        print("SERVER CLOSE")
        pass

class ServerDiscoveryHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        data = self.request[0]
        socket = self.request[1]
        print("Discovery server: ", self.client_address, "-", data)
        socket.sendto("Hi from discovery server " + platform.node(), self.client_address)


class CGSyncHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        data = self.request.recv(1024)
        print("CGSync server: ", self.client_address, "-", data)
        self.request.sendall("Hi from cgsync server " + platform.node())
		
def startThreadedServer(s, name):
	print(">>> Starting " + name)
	s.serve_forever()
	print(">>> Stopping " + name)

HOST, PORT = "239.5.0.5", 1505
discoveryServer = ServerDiscoveryServer((HOST, PORT), ServerDiscoveryHandler)
discoveryThread = threading.Thread(target = startThreadedServer, args = (discoveryServer, "CGsync service discovery server"))
discoveryThread.daemon = True
discoveryThread.start()

cgsyncServer = SocketServer.TCPServer(("0.0.0.0", 2505), CGSyncHandler)
print(cgsyncServer.server_address)

startThreadedServer(cgsyncServer, "CGSync server")
