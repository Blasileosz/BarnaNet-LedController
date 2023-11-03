#include "B_tcpServer.h"

static void do_retransmit(const int sock)
{
	int len;
	char rx_buffer[128];

	do
	{
		len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
		if (len < 0)
		{
			ESP_LOGE(tcpTag, "Error occurred during receiving: errno %d", errno);
		}
		else if (len == 0)
		{
			ESP_LOGE(tcpTag, "Connection closed");
		}
		else
		{
			rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
			ESP_LOGI(tcpTag, "Received %d bytes: %s", len, rx_buffer);

			// send() can return less bytes than supplied length.
			// Walk-around for robust implementation.
			int to_write = len;
			while (to_write > 0)
			{
				B_tcpRecvCallback();
				int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
				if (written < 0)
				{
					ESP_LOGE(tcpTag, "Error occurred during sending: errno %d", errno);
					// Failed to retransmit, giving up
					return;
				}
				to_write -= written;
			}
		}
	} while (len > 0);
}

void B_tcpServerTask(void *pvParameters)
{
	char addr_str[128];
	int keepAlive = LWIP_TCP_KEEPALIVE;
	int keepIdle = 5;
	int keepInterval = 5;
	int keepCount = 3;

	struct sockaddr_in dest_addr = {
	    .sin_addr.s_addr = htonl(INADDR_ANY),
	    .sin_family = AF_INET,
	    .sin_port = htons(B_TCP_PORT)};

	// Create server socket
	int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listen_sock < 0)
	{
		ESP_LOGE(tcpTag, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return;
	}

	// Enable reuse address
	int opt = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ESP_LOGI(tcpTag, "Socket created");

	// Bind socket
	int err = bind(listen_sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
	if (err != 0)
	{
		ESP_LOGE(tcpTag, "Socket unable to bind: errno %d", errno);
		ESP_LOGE(tcpTag, "IPPROTO: %d", AF_INET);
		goto CLEAN_UP;
	}
	ESP_LOGI(tcpTag, "Socket bound, B_TCP_PORT %d", B_TCP_PORT);

	// Start listening
	err = listen(listen_sock, 1);
	if (err != 0)
	{
		ESP_LOGE(tcpTag, "Error occurred during listen: errno %d", errno);
		goto CLEAN_UP;
	}

	ESP_LOGI(tcpTag, "Socket listening");

	// Listening loop
	while (true)
	{
		struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
		socklen_t addr_len = sizeof(source_addr);
		int sock = accept(listen_sock, (struct sockaddr*)&source_addr, &addr_len);
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
		if (source_addr.ss_family == PF_INET)
		{
			inet_ntoa_r(((struct sockaddr_in*)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
		}

		if (source_addr.ss_family == PF_INET6)
		{
			inet6_ntoa_r(((struct sockaddr_in6*)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
		}

		ESP_LOGI(tcpTag, "Socket accepted ip address: %s", addr_str);

		do_retransmit(sock);

		shutdown(sock, 0);
		close(sock);
	}

CLEAN_UP:
	close(listen_sock);
	vTaskDelete(NULL);
}
