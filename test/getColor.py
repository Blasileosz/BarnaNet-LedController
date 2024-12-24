import socket

HOST = '192.168.0.80'
PORT = 2005

tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.connect((HOST, PORT))

data = [0b01000000, 0b00000000]
print(data)
tcp_socket.sendall(bytes(data))

response = tcp_socket.recv(1024)
print("Received:", list(response))

tcp_socket.close()
