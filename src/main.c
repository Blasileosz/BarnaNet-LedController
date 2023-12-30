#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>

#include <nvs_flash.h>

#include "B_wifi.h"
#include "B_tcpServer.h"
#include "B_LedController.h"
#include "B_lightCommandStruct.h"

static const char *tag = "BarnaNet";

QueueHandle_t commandQueue;

void app_main()
{
	// Init NVS
	esp_err_t flashReturn = nvs_flash_init();
	if (flashReturn == ESP_ERR_NVS_NO_FREE_PAGES || flashReturn == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		flashReturn = nvs_flash_init();
	}
	ESP_ERROR_CHECK(flashReturn);

	// Connect to WIFI
	if (B_WifiConnect() != B_WIFI_OK)
	{
		ESP_LOGE(tag, "Wifi failed");
		return;
	}

	// Cretate command queue for maximum 10 commands
	commandQueue = xQueueCreate(10, sizeof(B_command_t));
	if (commandQueue == NULL)
	{
		ESP_LOGE(tag, "Command Queue could not be created");
		return;
	}

	if (!B_StartTCPServer()) {
		ESP_LOGE(tag, "TCP server failed");
		return;
	}

	// Create TCP server task
	xTaskCreate(B_ListenTCPServer, "balaTCPListen", 4096, &commandQueue, 3, NULL);
	xTaskCreate(B_LedControllerTask, "balaLedController", 2048, &commandQueue, 3, NULL);
}
