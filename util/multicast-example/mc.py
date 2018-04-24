import socket, time, sys, threading

ANY = "0.0.0.0"
SENDERPORT = 1601
MCAST_ADDR = "239.5.0.5"
MCAST_PORT = 1505

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
#The sender is bound on (0.0.0.0:1501)
for i in range(1, 10):
	try:
		sock.bind((ANY,SENDERPORT))
	except socket.error:
		SENDERPORT += 1
		continue
	break
		
print "Created socket at %s:%d" % (ANY, SENDERPORT)

#Tell the kernel that we want to multicast and that the data is sent
#to everyone (255 is the level of multicasting)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 30)

sock.sendto("Hi from python client: " + sys.argv[1], (MCAST_ADDR,MCAST_PORT) );
sock.setblocking(0)

addrs = []

print ">>> Waiting for servers to respond..."
i = 0
while i < 20:
	time.sleep(0.1)
	#send the data "hello, world" to the multicast addr: port
	#Any subscribers to the multicast address will receive this data
	try:
		data, addr = sock.recvfrom(1024)
		print "RESPONSE: ", addr, " - ", data
		
		addrs.append(addr[0])
	except socket.error:
		i += 1

sock.close()
sock = None

for i in addrs:
	print "Connecting to TCP server at ", (i, 2505)
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
	sock.connect((i, 2505))
	sock.send("Hi TCP server, this is cgsync client")
	data = sock.recv(1024)
	sock.close()
	print "cgsync server responded:", data, "\n"
