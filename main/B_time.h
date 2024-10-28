// Handles NTP sync and timer alarms
// Alarms: https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html#_CPPv415timer_set_alarm13timer_group_t11timer_idx_t13timer_alarm_t

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <esp_netif_sntp.h>
#include <nvs_flash.h>

#include <sys/time.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

// Set timezone https://developer.ibm.com/articles/au-aix-posix/
// Timezone:	CET (DST off)
//			CEST (DST on)
// UTC Offset:	+1
// CEST	start:	Last Sunday of March  at 02:00 AM
//		end:		Last Sunday of October at 03:00 AM
// (assuming 5 sundays in a month)

#define B_TIMEZONE "CET-1CEST,M3.5.0/2:00:00,M10.5.0/3:00:00"
#define B_SNTP_MAX_RETRY 15
#define B_SNTP_TIMEOUT_MS 2000

static const char* timeTag = "BarnaNet - TIME";

void B_SyncTime();
void B_DeinitSntp();