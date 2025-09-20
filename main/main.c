#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>

#include <nvs_flash.h>

#include "B_wifi.h"
#include "B_time.h"
#include "B_mqtt.h"
#include "B_alarm.h"
#include "B_tcpServer.h"
#include "B_ledController.h"
#include "B_BarnaNetCommand.h"

#define B_BUILTIN_LED 2

static const char *tag = "BarnaNet";

B_addressMap_t addressMap = { 0 };

struct B_TCPIngressTaskParameter tcpIngressTaskParameter = { 0 };
struct B_AlarmTaskParameter alarmTaskParameter = { 0 };
struct B_MQTTTaskParameter mqttTaskParameter = { 0 };
struct B_LedControllerTaskParameter ledControllerTaskParameter = { 0 };

void app_main()
{
	// Init NVS
	esp_err_t flashReturn = nvs_flash_init();
	if (flashReturn == ESP_ERR_NVS_NO_FREE_PAGES || flashReturn == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		ESP_ERROR_CHECK(nvs_flash_erase());
		ESP_LOGI(tag, "NVS partition got corrupted, erased it");
		flashReturn = nvs_flash_init();
	}
	ESP_ERROR_CHECK(flashReturn);

	// Set up boot light
	ESP_ERROR_CHECK(gpio_reset_pin(B_BUILTIN_LED));
	ESP_ERROR_CHECK(gpio_set_direction(B_BUILTIN_LED, GPIO_MODE_DEF_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(B_BUILTIN_LED, 1));
	ESP_LOGI(tag, "GPIO is on for pin: %i", B_BUILTIN_LED);

	// Connect to WIFI
	if (B_WifiConnect() != B_WIFI_OK) {
		ESP_LOGE(tag, "Wifi failed");
		return;
	}

	// NTP
	if (!B_SyncTime()){
		ESP_LOGE(tag, "Failed to get time");
		// Not restarting device, because it may get into a loop
		//esp_restart();
		return;
	}
	B_PrintLocalTime();

	// Create command queues (maximum 10 commands)
	QueueHandle_t tcpCommandQueue = xQueueCreate(10, sizeof(B_command_t));
	QueueHandle_t alarmCommandQueue = xQueueCreate(10, sizeof(B_command_t));
	QueueHandle_t mqttCommandQueue = xQueueCreate(10, sizeof(B_command_t));
	QueueHandle_t ledCommandQueue = xQueueCreate(10, sizeof(B_command_t));
	if (tcpCommandQueue == NULL || alarmCommandQueue == NULL || mqttCommandQueue == NULL || ledCommandQueue == NULL) {
		ESP_LOGE(tag, "Failed to create queues");
		B_SntpCleanup();
		return;
	}

	// Address Map
	if (!B_AddressMapInit(&addressMap, 4)) {
		ESP_LOGE(tag, "Failed to create address map");
		B_SntpCleanup();

		// Manually delete the command queues
		vQueueDelete(tcpCommandQueue);
		vQueueDelete(alarmCommandQueue);
		vQueueDelete(mqttCommandQueue);
		vQueueDelete(ledCommandQueue);
		return;
	}

	// Insert queues into the address map
	B_InsertAddress(&addressMap, 0, B_TASKID_TCP, tcpCommandQueue, B_TASK_FLAG_ONLY_REPLY);
	B_InsertAddress(&addressMap, 1, B_TASKID_ALARM, alarmCommandQueue, B_TASK_FLAG_NO_REPLY);
	B_InsertAddress(&addressMap, 2, B_TASKID_MQTT, mqttCommandQueue, B_TASK_FLAG_ONLY_REPLY);
	B_InsertAddress(&addressMap, 3, B_TASKID_LED, ledCommandQueue, 0);

	// Prepare TCP task parameters
	tcpIngressTaskParameter.addressMap = &addressMap;

	// Prepare MQTT task parameters
	mqttTaskParameter.addressMap = &addressMap;

	// Prepare alarm task parameters
	alarmTaskParameter.addressMap = &addressMap;

	// Prepare led task parameters
	ledControllerTaskParameter.addressMap = &addressMap;

	xTaskCreate(B_TCPIngressTask, "B_TCPIngressTask", 1024 * 4, &tcpIngressTaskParameter, 3, NULL);
	xTaskCreate(B_AlarmTask, "B_AlarmTask", 1024 * 3, &alarmTaskParameter, 3, NULL);
	xTaskCreate(B_LedControllerTask, "B_LedControllerTask", 1024 * 4, &ledControllerTaskParameter, 3, NULL);
	xTaskCreate(B_MQTTTask, "B_MQTTTask", 1024 * 4, &mqttTaskParameter, 3, NULL);

	// Turn off boot light
	ESP_ERROR_CHECK(gpio_set_level(B_BUILTIN_LED, 0));

	ESP_LOGI(tag, "Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
	ESP_LOGI(tag, "IDF version: %s", esp_get_idf_version());

	// A task should never return (when should the systems be cleaned up?)
	vTaskDelete(NULL);

	//B_SntpCleanup();
	//B_WifiDisconnect();
	//B_WifiCleanup();
	//B_AddressmapCleanup(&addressMap);
}
