#include "B_time.h"

void B_SyncTime()
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_LOGI(timeTag, "Initializing SNTP");

	esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
	config.start = true; // Automatically starts
	esp_netif_sntp_init(&config); // Starts time sync

	// Wait for time sync event
	int retry = 0;
	while (esp_netif_sntp_sync_wait(B_SNTP_TIMEOUT_MS / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < B_SNTP_MAX_RETRY)
	{
		ESP_LOGI(timeTag, "Waiting for system time to be set... (%i/%i)", retry, B_SNTP_MAX_RETRY);
	}

	// Set system timezone
	setenv("TZ", B_TIMEZONE, true);
	tzset();
}

void B_DeinitSntp()
{
	//esp_netif_sntp_deinit();
}
