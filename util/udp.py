import socket

UDP_IP = "192.168.1.109"
UDP_PORT = 51000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

print "Listening now..."
while True:
	message, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
	print "received message:", message, "from", addr
	
	client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
	
	reply = ""
	if message == "Hi.":
		reply = "Hello."
	elif message == "How are you?":
		reply = "I'm fantastic, thanks for asking."
	else:
		reply = "Yo, yo, yo! My IP is: " + socket.gethostbyname(socket.gethostname())
	print "reply:", reply
	client.sendto(reply, addr)
	