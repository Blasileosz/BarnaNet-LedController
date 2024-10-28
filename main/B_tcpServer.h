#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

#include "B_SECRET.h"
#include "B_lightCommandStruct.h"

static const char* tcpTag = "BarnaNet - TCP";

static int tcpSock = 0;

// Reads the message and transmits a response
static void B_HandleTCPMessage(const int sock, QueueHandle_t* commandQueuePtr);
static void B_TCPSendMessage(int sock, char* data, size_t size);

// Inits the TCP server
bool B_StartTCPServer();

// Creates a TCP server and listens for messages (possibly used in a separate task)
void B_ListenTCPServer(void* pvParameters);
