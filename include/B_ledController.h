#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>

#include <driver/gpio.h> // Basic GPIO control
#include <driver/ledc.h> // PWM control

#include "B_SECRET.h"
#include "B_lightCommandStruct.h"

#define LED_UPDATE_HZ 40

#define LED_STATE_FINISHED 0
#define LED_STATE_INPROGRESS 1
//#define LED_STATE_TRANSITION 2

static const char* ledControllerTag = "BarnaNet - LedController";

typedef struct {
	uint8_t red, green, blue;
} B_color_t;

// Linear Interpolate between colors
void B_ColorLerp(B_color_t* a, B_color_t* b, float t, B_color_t* out);

typedef struct {
	uint8_t state;
	uint16_t timer; // miliseconds
	B_color_t previousColor;
} B_commandExecution_t;

void B_SetUpPwmChanels();
void B_SetPWMColor(B_color_t* color);

// The function registered as a task that handles the messages from the TCP server
void B_LedControllerTask(void* pvParameters);

// GPIO
void B_SetUpGpioPin(gpio_num_t pin);
void B_SetGpioMode(gpio_num_t pin, bool mode);
void B_GPIORotateColorExample();
