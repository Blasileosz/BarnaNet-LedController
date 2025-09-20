import socket
from defines import *

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.settimeout(5)
tcp_socket.connect((HOST, PORT))

colorChange = [0, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_COLOR, B_PLACEHOLDER_TID, 69, 255, 19, 0b00000100, 0b00000000]
function = [0, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_FUNCTION, B_PLACEHOLDER_TID, 2, 0b10000000, 0b00000000]

data = [0, B_COMMAND_DEST_ALARM, B_COMMAND_OP_SET | B_ALARM_COMMAND_INSERT, B_PLACEHOLDER_TID, 0, 0, 0, 0, B_EVERYDAY]

timepart = GetTimepart(13, 00, 00)
print("Timepart:", timepart, SerializeDWORD(timepart))
data[3:7] = SerializeDWORD(timepart)

print(data + function)
tcp_socket.sendall(bytes(data + function))

response = tcp_socket.recv(1024)
print("Received:", list(response))
responseBody = response[4:]
print(''.join(map(chr, responseBody)))

tcp_socket.close()
