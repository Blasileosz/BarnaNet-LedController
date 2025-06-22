import socket
from defines import *

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.settimeout(5)
tcp_socket.connect((HOST, PORT))

# COLOR: [B_COMMAND_OP_SET | B_COMMAND_DEST_LED, B_LED_COMMAND_COLOR, unused, RED, GREEN, BLUE, TIME_HIGHPART, TIME_LOWPART]
# FUNCTION: [B_COMMAND_OP_SET | B_COMMAND_DEST_LED, B_LED_COMMAND_FUNCION, unused, FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART]
data = [B_COMMAND_OP_SET | B_COMMAND_DEST_LED, B_LED_COMMAND_COLOR, 0, 69, 255, 19, 0b00000100, 0b00000000]
#data = [B_COMMAND_OP_SET | B_COMMAND_DEST_LED, B_LED_COMMAND_FUNCION, 0, 0, 0b00000100, 0b00000000]
print(data)
tcp_socket.sendall(bytes(data))

response = tcp_socket.recv(1024)
print("Received:", list(response))

tcp_socket.close()
