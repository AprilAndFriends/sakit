import socket

MCAST_GROUP = "226.2.3.4"
MCAST_PORT = 50300

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 32)
print(sock.sendto("This is a test", (MCAST_GROUP, MCAST_PORT)))