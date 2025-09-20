#pragma once

#define B_TASKID_LED 11

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>

#include <nvs_flash.h>

#include <lwip/sockets.h>

#include <driver/gpio.h> // Basic GPIO control
#include <driver/ledc.h> // PWM control

#include "B_SECRET.h"
#include "B_BarnaNetCommand.h"
#include "B_colorUtil.h"

#define B_LED_NVS_NAMESPACE "B_LED"
#define B_LED_NVS_BUFFER "B_LED_BUFFER"

static_assert(CONFIG_B_RED_PIN != CONFIG_B_GREEN_PIN);
static_assert(CONFIG_B_RED_PIN != CONFIG_B_BLUE_PIN);
static_assert(CONFIG_B_GREEN_PIN != CONFIG_B_BLUE_PIN);

#define B_LED_UPDATE_HZ 40

enum B_LED_COMMAND_IDS {
	B_LED_COMMAND_STATE,
	B_LED_COMMAND_COLOR,
	B_LED_COMMAND_FUNCTION
};

enum B_LED_FUNCTIONS {
	B_LED_FUNCTION_NONE, // Also acts as the color transition state
	B_LED_FUNCTION_RAINBOW1,
	B_LED_FUNCTION_RAINBOW2,
	B_LED_FUNCTION_ENUM_SIZE
};

struct B_ledState {
	bool isOn;

	uint8_t functionID; // Also stores the color transition state
	uint16_t functionSpeed; // Time needed to complete one cycle of the function in ms (In case of color transition, acts as the transition time)

	B_color_t color; // If on a gradual function (like rainbow), this should follow the color of the strip, to be able to transition from that when a color command is received
	B_color_t previousColor;

	uint16_t timer; // milliseconds
};

// void B_SetUpPwmChannels()
// Creates a LEDC timer and three channels for the PWM module
// Arguments: void
// Returns: void
// - Private function
// - !Runs in the LED task

// void B_SetPWMColor(const B_color_t* const);
// Changes color of the LEDs
// Arguments: color: the color passed to the PWM module
// Returns: void
// - Private function
// - !Runs in the LED task

// void B_LoadLedStateFromNVS()
// Loads the LED state from NVS
// Arguments: void
// Returns: void
// - Private function
// - !Runs in the LED task

// void B_SaveLedStateToNVS()
// Saves the LED state to NVS
// Arguments: void
// Returns: void
// - Private function
// - !Runs in the LED task

// TickType_t B_ColorTransitionRenderer()
// Handles the color transition state
// Arguments: void
// Returns: A TickType_t that specifies how long the task should sleep between two updates (portMAX_DELAY signals that the transition finished and the task must not update the leds anymore)
// - Private function
// - !Runs in the LED task

// TickType_t B_RainbowFunction1Renderer()
// Updates and renders the rainbow function, same system can be used for other gradual functions
// Arguments: void
// Returns: A TickType_t that specifies how long the task should sleep between two updates (portMAX_DELAY signals that the transition finished and the task must not update the leds anymore)
// - Private function
// - !Runs in the LED task

// TickType_t B_RainbowFunction2Renderer()
// Should do the same as B_RainbowFunction1Renderer(), but uses HSLtoRGB
// TODO: Can we use 3 sine waves to create a simple hsl to rgb function?
// Arguments: void
// Returns: A TickType_t that specifies how long the task should sleep between two updates (portMAX_DELAY signals that the transition finished and the task must not update the leds anymore)
// - Private function
// - !Runs in the LED task

struct B_LedControllerTaskParameter {
	B_addressMap_t* addressMap;
};

// Updates the leds
// - !Runs in the LED Task
// - Expected parameter: B_LedControllerTaskParameter struct
void B_LedControllerTask(void* pvParameters);

// GPIO
void B_SetUpGpioPin(gpio_num_t pin);
void B_SetGpioMode(gpio_num_t pin, bool mode);
void B_GPIORotateColorExample();
