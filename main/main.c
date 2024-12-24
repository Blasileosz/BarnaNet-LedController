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
#include "B_protocolCommands.h"
#include "B_globalState.h"

#define B_BUILTIN_LED 2

static const char *tag = "BarnaNet";

B_globalState_t globalState = { 0 };
SemaphoreHandle_t stateMutex;

// Respond or consume the commands sent via TCP
// - !Runs in the TCP task
static void B_HandleTCPMessage(const char* const messageBuffer, int messageLen, char* const responseBufferOut, int* const responseLenOut)
{
	if (messageLen > B_COMMAND_MAX_LEN) {
		ESP_LOGE(tag, "Received invalid command");
		responseBufferOut[0] = B_ERROR_COMMAND_MASK;
		*responseLenOut = 1;
		return;
	}

	const B_command_t* const command = (const B_command_t* const)messageBuffer;
	switch (command->commandID) {
		case B_COMMAND_SETCOLOR:
			{
				// TODO: Could create a command struct specifically for color and function
				xSemaphoreTake(stateMutex, portMAX_DELAY);

				globalState.isOn = true;
				globalState.color.red = command->data[0];
				globalState.color.green = command->data[1];
				globalState.color.blue = command->data[2];
				globalState.functionID = 0;
				globalState.functionSpeed = htonl(*((uint16_t*)(&command->data[3])));

				xSemaphoreGive(stateMutex);
				return;
			}

			case B_COMMAND_SETFUNCTION:
			{
				xSemaphoreTake(stateMutex, portMAX_DELAY);

				globalState.isOn = true;
				globalState.functionID = command->data[0];
				globalState.functionSpeed = htonl(*((uint16_t*)(&command->data[1])));

				xSemaphoreGive(stateMutex);
				return;
			}

			case B_COMMAND_GETCOLOR:
			{
				xSemaphoreTake(stateMutex, portMAX_DELAY);

				// TODO: Could construct a command_t and memcopy it into the response

				responseBufferOut[0] = B_COMMAND_RESPONDCOLOR;
				responseBufferOut[1] = 0;
				responseBufferOut[2] = globalState.color.red;
				responseBufferOut[3] = globalState.color.green;
				responseBufferOut[4] = globalState.color.blue;
				*responseLenOut = 5;

				xSemaphoreGive(stateMutex);
				return;
			}

		default:
			break;
	}
}

void app_main()
{
	static_assert(sizeof(B_command_t) == B_COMMAND_MAX_LEN);

	// Init NVS
	esp_err_t flashReturn = nvs_flash_init();
	if (flashReturn == ESP_ERR_NVS_NO_FREE_PAGES || flashReturn == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		flashReturn = nvs_flash_init();
	}
	ESP_ERROR_CHECK(flashReturn);

	// DEBUG Light
	gpio_reset_pin(B_BUILTIN_LED);
	ESP_ERROR_CHECK(gpio_set_direction(B_BUILTIN_LED, GPIO_MODE_DEF_OUTPUT));
	ESP_LOGI(tag, "GPIO is on for pin: %i", B_BUILTIN_LED);

	// Connect to WIFI
	if (B_WifiConnect() != B_WIFI_OK) {
		ESP_LOGE(tag, "Wifi failed");
		return;
	}

	// DEBUG
	ESP_ERROR_CHECK(gpio_set_level(B_BUILTIN_LED, 1));

	// NTP
	B_SyncTime();
	B_PrintLocalTime();

	// Create mutex for locking globalState
	stateMutex = xSemaphoreCreateMutex();
	if (stateMutex == NULL) {
		ESP_LOGE(tag, "Failed to create mutex");
		return;
	}

	xTaskCreate(B_TCPTask, "B_TCPTask", 4096, &B_HandleTCPMessage, 3, NULL);
	//xTaskCreate(&B_LedControllerTask, "B_LedControllerTask", 2048, &stateMutex, 3, NULL);

	//B_DeinitSntp();
}
