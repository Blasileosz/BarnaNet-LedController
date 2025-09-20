import socket
from defines import *

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.settimeout(5)
tcp_socket.connect((HOST, PORT))

data = [0, B_COMMAND_DEST_ALARM, B_COMMAND_OP_SET | B_ALARM_COMMAND_REMOVE, B_PLACEHOLDER_TID, 0]

print(data)
tcp_socket.sendall(bytes(data))

response = tcp_socket.recv(1024)
print("Received:", list(response))
responseBody = response[4:]
print(''.join(map(chr, responseBody)))

tcp_socket.close()
