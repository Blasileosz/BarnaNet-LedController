#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>

#include <lwip/sockets.h>

#include <driver/gpio.h> // Basic GPIO control
#include <driver/ledc.h> // PWM control

#include "B_SECRET.h"
#include "B_BarnaNetCommand.h"
#include "B_colorUtil.h"

static_assert(CONFIG_B_RED_PIN != CONFIG_B_GREEN_PIN);
static_assert(CONFIG_B_RED_PIN != CONFIG_B_BLUE_PIN);
static_assert(CONFIG_B_GREEN_PIN != CONFIG_B_BLUE_PIN);

#define B_LED_UPDATE_HZ 40

// -- COMMANDS -- //
#define B_LED_COMMAND_STATE 0
#define B_LED_COMMAND_COLOR 1
#define B_LED_COMMAND_FUNCION 2

// EXAMPLES:

// GET STATE - Expected data: none
// - Example: [B_COMMAND_OP_GET | B_COMMAND_DEST_LED, B_LED_COMMAND_STATE, unused]
// RESPOND STATE - Response
// - bytes 0-2: r,g,b
// - Example: [B_COMMAND_OP_RES | B_COMMAND_DEST_LED, B_LED_COMMAND_STATE, unused, STATUS, FUNCTION_ID, FUNCTION_SPEED_HIGH, FUNCTION_SPEED_LOW, RED, GREEN, BLUE]

// SET COLOR - Expected data
// - bytes 0-2: r,g,b
// - bytes 3-4: lerp time (ms)
// - Example: [B_COMMAND_OP_SET | B_COMMAND_DEST_LED, B_LED_COMMAND_COLOR, unused, RED, GREEN, BLUE, TIME_HIGHPART, TIME_LOWPART]

// SET FUNCTION - Expected data
// - bytes 0: function id
// - bytes 1-2: speed (ms)
// - Example: [B_COMMAND_OP_SET | B_COMMAND_DEST_LED, B_LED_COMMAND_FUNCION, unused, FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART]

// -- FUNCTIONS -- //
#define B_LED_FUNCTION_NONE 0 // Also acts as the color transition state
#define B_LED_FUNCTION_RAINBOW1 1
#define B_LED_FUNCTION_RAINBOW2 2


struct B_ledState {
	bool isOn;

	uint8_t functionID; // Also stores the color transition state
	uint16_t functionSpeed; // Time needed to complete one cycle of the function in ms (In case of color transition, acts as the transition time)

	B_color_t color; // If on a gradual function (like rainbow), this should follow the color of the strip, to be able to transition from that when a color command is received
	B_color_t previousColor;

	uint16_t timer; // miliseconds
};

// void B_SetUpPwmChanels()
// Creates a LEDC timer and three chanels for the PWM module
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

struct B_LedControllerTaskParameter{
	QueueHandle_t* tcpCommandQueue;
	QueueHandle_t* ledCommandQueue;
};

// Updates the leds
// - !Runs in the LED Task
void B_LedControllerTask(void* pvParameters);

// GPIO
void B_SetUpGpioPin(gpio_num_t pin);
void B_SetGpioMode(gpio_num_t pin, bool mode);
void B_GPIORotateColorExample();
