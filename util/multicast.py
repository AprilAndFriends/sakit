import socket
import struct

MCAST_GROUP = "226.2.3.4"
MCAST_PORT = 50300
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', MCAST_PORT))
group = struct.pack("4sl", socket.inet_aton(MCAST_GROUP), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, group)

print("Listening now...")
while True:
	message, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
	print("received message:", message, "from", addr)
	
	#client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
	
	reply = ""
	if message == "Hi.":
		reply = "Hello."
	elif message == "How are you?":
		reply = "I'm fantastic, thanks for asking."
	else:
		reply = "Yo, yo, yo! My IP is: " + socket.gethostbyname(socket.gethostname())
	print("reply:", reply)
	sock.sendto(reply, addr)
