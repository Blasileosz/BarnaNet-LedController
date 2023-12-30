#include "B_tcpServer.h"

static void B_HandleTCPMessage(const int sock, QueueHandle_t* commandQueuePtr)
{
	int len;
	char receiveBuffer[128] = {0};

	while (true) {
		len = recv(sock, receiveBuffer, sizeof(receiveBuffer), 0);
		if (len < 0)
		{
			ESP_LOGE(tcpTag, "Error occurred during receiving: errno %d", errno);
			return;
		}

		if (len == 0)
		{
			ESP_LOGE(tcpTag, "Connection closed");
			return;
		}

		ESP_LOGI(tcpTag, "Received %d bytes", len);

		// Prepare command to insert to queue
		if (len > sizeof(B_command_t)){
			ESP_LOGE(tcpTag, "Received more data than command struct is capeable of hadling");
			return;
		}

		B_command_t insertCommand;
		memcpy(&insertCommand, receiveBuffer, sizeof(B_command_t));

		// Put message into command queue
		if (xQueueSend(*commandQueuePtr, &insertCommand, 0) != pdPASS)
		{
			ESP_LOGE(tcpTag, "Could not insert message into command queue: errno %d", errno);
		}

		ESP_LOGI(tcpTag, "Inserted command to queue");
	}
}

static void B_TCPSendMessage(int sock, char* data, size_t size)
{
	// Send data in chunks if not able to send it in one go
	size_t to_write = size;
	while (to_write > 0)
	{
		int written = send(sock, data + (size - to_write), to_write, 0);
		if (written < 0)
		{
			ESP_LOGE(tcpTag, "Error occurred during sending: errno %d", errno);
			return;
		}
		to_write -= written;
	}
}

bool B_StartTCPServer()
{
	struct sockaddr_in dest_addr = {
		.sin_addr.s_addr = htonl(INADDR_ANY),
		.sin_family = AF_INET,
		.sin_port = htons(B_TCP_PORT)
	};

	// Create server socket
	tcpSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (tcpSock < 0)
	{
		ESP_LOGE(tcpTag, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return false;
	}

	// Enable reuse address
	int opt = 1;
	setsockopt(tcpSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ESP_LOGI(tcpTag, "Socket created");

	// Bind socket
	int err = bind(tcpSock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (err != 0)
	{
		ESP_LOGE(tcpTag, "Socket unable to bind: errno %d", errno);
		ESP_LOGE(tcpTag, "IPPROTO: %d", AF_INET);
		close(tcpSock);
		return false;
	}
	ESP_LOGI(tcpTag, "Socket bound %d", B_TCP_PORT);

	// Start listening
	err = listen(tcpSock, 1);
	if (err != 0)
	{
		ESP_LOGE(tcpTag, "Error occurred during listen: errno %d", errno);
		close(tcpSock);
		return false;
	}

	ESP_LOGI(tcpTag, "Socket listening");
	return true;
}

void B_ListenTCPServer(void* pvParameters)
{
	QueueHandle_t* commandQueuePtr = (QueueHandle_t*)pvParameters;

	char addr_str[128];
	int keepAlive = LWIP_TCP_KEEPALIVE;
	int keepIdle = 5;
	int keepInterval = 5;
	int keepCount = 3;

	// Listening loop
	while (true)
	{
		struct sockaddr_storage clientAddress; // Large enough for both IPv4 or IPv6
		socklen_t clientAddrlen = sizeof(clientAddress);
		int sock = accept(tcpSock, (struct sockaddr *)&clientAddress, &clientAddrlen);
		if (sock < 0)
		{
			ESP_LOGE(tcpTag, "Unable to accept connection: errno %d", errno);
			break;
		}

		// Set tcp keepalive option
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

		// Convert ip address to string
		if (clientAddress.ss_family == PF_INET)
		{
			inet_ntoa_r(((struct sockaddr_in *)&clientAddress)->sin_addr, addr_str, sizeof(addr_str) - 1);
		}

		if (clientAddress.ss_family == PF_INET6)
		{
			inet6_ntoa_r(((struct sockaddr_in6 *)&clientAddress)->sin6_addr, addr_str, sizeof(addr_str) - 1);
		}

		ESP_LOGI(tcpTag, "Socket accepted ip address: %s", addr_str);

		B_HandleTCPMessage(sock, commandQueuePtr);

		close(sock);
	}

	close(tcpSock);
	vTaskDelete(NULL);
}
