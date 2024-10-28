import socket

HOST = '192.168.0.80'
PORT = 2005

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.connect((HOST, PORT))

# { 0b00000000, 0b00000000, RED_VALUE(color), GREEN_VALUE(color), BLUE_VALUE(color), 0b00000100, 0b00000000};
data = [0b00000000, 0b00000000, 64, 0, 0, 0b00000100, 0b00000000]
print(data)
tcp_socket.sendall(bytes(data))

response = tcp_socket.recv(1024)
print("Received:", response.decode())

tcp_socket.close()
