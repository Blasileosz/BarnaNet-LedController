import socket

# // COMMAND HEADER:	0b11222222
# // COMMAND ID:		0b33333333
# // 1: Command operation (Get, Set, Response, Error)
# // 2: Command destination (LED controller, Alarm system, etc.)
# // 3: Command id (Unique identifier for a command given the operation and destination)

B_COMMAND_OP_MASK = 0b11000000
B_COMMAND_OP_SET = 0b00000000
B_COMMAND_OP_GET = 0b01000000
B_COMMAND_OP_RES = 0b10000000
B_COMMAND_OP_ERR = 0b11000000

B_COMMAND_DEST_MASK = 0b00111111
B_COMMAND_DEST_LED = 1
B_COMMAND_DEST_ALARM = 2

B_LED_COMMAND_STATE = 0 # Only get
B_LED_COMMAND_COLOR = 1 # Only set
B_LED_COMMAND_FUNCION = 2 # Only set

HOST = '192.168.0.80'
PORT = 2005

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
