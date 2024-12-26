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

QueueHandle_t tcpCommandQueue = { 0 };
QueueHandle_t ledCommandQueue = { 0 };

B_LedControllerTaskParameter_t ledControllerTaskParameter = { 0 };

// Dispatches the commands sent via TCP
// - !Runs in the TCP task
static void B_HandleTCPMessage(const B_command_t* const command, B_command_t* const responseCommand)
{
	static const char* tcpHandlerTag = "BarnaNet - TCP Handler";

	// Sanitize request type
	if (B_COMMAND_OP(command->header) == B_COMMAND_OP_RES || B_COMMAND_OP(command->header) == B_COMMAND_OP_ERR) {
		ESP_LOGE(tcpHandlerTag, "Invalid request type");
		responseCommand->header = B_COMMAND_OP_ERR | B_COMMAND_DEST(command->header);
		responseCommand->ID = command->ID;
		responseCommand->data[0] = B_COMMAND_ERR_CLIENT;
		return;
	}

	// Command for the LED controller
	if (B_COMMAND_DEST(command->header) == B_COMMAND_DEST_LED) {

		// Send the command off to the led controller to process
		if (xQueueSend(ledCommandQueue, (void* const)command, 0) != pdPASS) {
			ESP_LOGE(tcpHandlerTag, "Command unable to be inserted into the queue");
			responseCommand->header = B_COMMAND_OP_ERR | B_COMMAND_DEST(command->header);
			responseCommand->ID = command->ID;
			responseCommand->data[0] = B_COMMAND_ERR_INTERNAL;
			return;
		}

		// GET command, must wait for reply
		if (B_COMMAND_OP(command->header) == B_COMMAND_OP_GET) {

			if (xQueueReceive(tcpCommandQueue, (void* const)responseCommand, portTICK_PERIOD_MS * 1000) != pdPASS) {
				ESP_LOGE(tcpHandlerTag, "LED Controller did not reply to get request");
				responseCommand->header = B_COMMAND_OP_ERR | B_COMMAND_DEST(command->header);
				responseCommand->ID = command->ID;
				responseCommand->data[0] = B_COMMAND_ERR_INTERNAL;
				return;
			}

			// Send back to client (fill header just in case)
			responseCommand->header = B_COMMAND_OP_RES | B_COMMAND_DEST(command->header);
			responseCommand->ID = command->ID;
			responseCommand->stripID = command->stripID;
			return;
		}
	}

	// Handle invalid DEST
}

void app_main()
{
	static_assert(sizeof(B_command_t) == 128);

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

	// Create command queues for the tcp sever and the led controller (maximum 10 commands)
	tcpCommandQueue = xQueueCreate(10, sizeof(B_command_t));
	ledCommandQueue = xQueueCreate(10, sizeof(B_command_t));
	if (tcpCommandQueue == NULL || ledCommandQueue == NULL) {
		ESP_LOGE(tag, "Failed to create queues");
		B_DeinitSntp();
		return;
	}

	// Preapare led task parameters
	ledControllerTaskParameter.tcpCommandQueue = &tcpCommandQueue;
	ledControllerTaskParameter.ledCommandQueue = &ledCommandQueue;

	xTaskCreate(B_TCPTask, "B_TCPTask", 1024 * 4, &B_HandleTCPMessage, 3, NULL);
	xTaskCreate(B_LedControllerTask, "B_LedControllerTask", 1024 * 4, &ledControllerTaskParameter, 3, NULL);

	//B_DeinitSntp();
}
