from socket import *
sock = socket(AF_INET, SOCK_STREAM)
sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

message = "POST http://mbattles.cateia.com/debug/mb.php HTTP/1.1\r\n\
Content-Length: 108\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Accept: */*\r\n\
Accept-Encoding: identity\r\n\
Connection: close\r\n\
Host: mbattles.cateia.com\r\n\
User-Agent: mbattles/1.7.2.0 (Windows)\r\n\
X-App-Check: DFD8CEA7\r\n\
X-Http-Proto: HTTP/1.1\r\n\
X-Real-Ip: 212.92.202.171\r\n\
\r\n\
vc=52CFBD39&action=login&username=5465737432&password=E0AF8F5928F7D7A1703F1FE9B8876731&device_id=&apn_token=\r\n\
\r\n"

sock.connect(('mbattles.cateia.com', 80))
print("Sent: " + str(sock.send(message)))
print("")

while True:
	data = sock.recv(2000)
	if data != "":
		print(data)
		break;


