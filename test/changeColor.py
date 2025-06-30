import socket
from defines import *

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.settimeout(5)
tcp_socket.connect((HOST, PORT))

# COLOR: [FROM, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_COLOR, RED, GREEN, BLUE, TIME_HIGHPART, TIME_LOWPART]
# FUNCTION: [FROM, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_FUNCTION, FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART]
data = [0, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_COLOR, 25, 0, 25, 0b00001000, 0b00000000]
#data = [0, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_FUNCTION, 2, 0b10000000, 0b00000000]
print(data)
tcp_socket.sendall(bytes(data))

response = tcp_socket.recv(1024)
print("Received:", list(response))
responseBody = response[3:]
print(''.join(map(chr, responseBody)))

tcp_socket.close()
