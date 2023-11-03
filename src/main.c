#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>

#include <nvs_flash.h>

// #include <lwip/err.h>
// #include <lwip/sockets.h>
// #include <lwip/sys.h>
// #include <lwip/netdb.h>
// #include <lwip/dns.h>

#include "B_wifi.h"
#include "B_tcpServer.h"

static const char* tag = "BarnaNet";

int B_tcpRecvCallback() {
	return 2;
}

void app_main() {
	// Init NVS
	esp_err_t flashReturn = nvs_flash_init();
	if (flashReturn == ESP_ERR_NVS_NO_FREE_PAGES || flashReturn == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		flashReturn = nvs_flash_init();
	}
	ESP_ERROR_CHECK(flashReturn);

	// Connect to WIFI
	if (B_WifiConnect() != B_WIFI_OK){
		ESP_LOGI(tag, "Wifi failed");
		return;
	}

	// Create TCP server task
	xTaskCreate(B_tcpServerTask, "tcpServer", 4096, NULL, 5, NULL);
}
