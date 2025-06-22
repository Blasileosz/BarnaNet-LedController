import socket
from defines import *

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.settimeout(1)
tcp_socket.connect((HOST, PORT))

# [B_COMMAND_OP_GET | B_COMMAND_DEST_LED, B_LED_COMMAND_STATE, unused]
data = [B_COMMAND_OP_GET | B_COMMAND_DEST_LED, B_LED_COMMAND_STATE, 0]
print(data)
tcp_socket.sendall(bytes(data))

response = tcp_socket.recv(1024)
print("Received:", list(response))

tcp_socket.close()
