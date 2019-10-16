import socket

TCP_IP = "192.168.1.109"
TCP_PORT = 25050
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # TCP
sock.bind((TCP_IP, TCP_PORT))
client = sock.listen(5)

print("Listening now...")
while True:
	client, addr = sock.accept()
	print("connected:", addr)
	message = client.recv(1024) # buffer size is 1024 bytes
	print("received message:", message)
	
	#client = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # UDP
	
	reply = ""
	if message == "Hi.":
		reply = "Hello."
	elif message == "How are you?":
		reply = "I'm fantastic, thanks for asking."
	else:
		reply = "Yo, yo, yo! My IP is: " + socket.gethostbyname(socket.gethostname())
	#reply = "Yo,yo,yo! MyIPis:"
	print("reply:", reply)
	#client.send("HandshakeOK")
	client.send(reply)
	