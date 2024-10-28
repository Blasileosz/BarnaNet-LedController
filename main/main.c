#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>

#include <nvs_flash.h>

#include "B_wifi.h"
#include "B_time.h"
#include "B_tcpServer.h"
#include "B_LedController.h"
#include "B_lightCommandStruct.h"

#define B_BUILTIN_LED 2

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

	// DEBUG Light
	gpio_reset_pin(B_BUILTIN_LED);
	ESP_ERROR_CHECK(gpio_set_direction(B_BUILTIN_LED, GPIO_MODE_DEF_OUTPUT));
	ESP_LOGI(ledControllerTag, "GPIO is on for pin: %i", B_BUILTIN_LED);

	// Connect to WIFI
	if (B_WifiConnect() != B_WIFI_OK)
	{
		ESP_LOGE(tag, "Wifi failed");
		return;
	}

	// DEBUG
	ESP_ERROR_CHECK(gpio_set_level(B_BUILTIN_LED, 1));

	// NTP
	B_SyncTime();

	time_t now = 0;
	struct tm timeinfo = { 0 };
	char timeBuffer[64];
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(timeBuffer, sizeof(timeBuffer), "%Y. %m. %d. - %X", &timeinfo);
	ESP_LOGI(tag, "New time: %s", timeBuffer);
	
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

	B_DeinitSntp();
}
