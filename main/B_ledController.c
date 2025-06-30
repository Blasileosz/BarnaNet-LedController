#include "B_ledController.h"

static const char* ledControllerTag = "BarnaNet - LedController";

static B_color_t rainbow[] = {
	{148, 0, 211},	// Violet
	{75, 0, 130},	// Indigo
	{0, 0, 255},	// Blue
	{0, 255, 0},	// Green
	{255, 255, 0},	// Yellow
	{255, 127, 0},	// Orange
	{255, 0, 0}	// Red
};

static struct B_ledState ledState = { 0 };


static void B_SetUpPwmChannels()
{
	ledc_timer_config_t timer = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_num = LEDC_TIMER_0,
		.duty_resolution = LEDC_TIMER_8_BIT,
		.freq_hz = 5000,
		.clk_cfg = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&timer));

	ledc_channel_config_t redChannel = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_0,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = CONFIG_B_RED_PIN,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&redChannel));

	ledc_channel_config_t greenChannel = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_1,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = CONFIG_B_GREEN_PIN,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&greenChannel));

	ledc_channel_config_t blueChannel = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_2,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = CONFIG_B_BLUE_PIN,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&blueChannel));

	ESP_LOGI(ledControllerTag, "Configured PWM module");
}

static void B_SetPWMColor(const B_color_t* const color)
{
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, color->red));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0));

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, color->green));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1));

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, color->blue));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2));
}

static TickType_t B_ColorTransitionRenderer()
{
	ledState.timer += (1000 / B_LED_UPDATE_HZ);

	B_color_t lerpedColor;
	B_ColorLerp(&ledState.previousColor, &ledState.color, (float)ledState.timer / (float)ledState.functionSpeed, &lerpedColor);

	B_SetPWMColor(&lerpedColor);

	// Finished, block till next command
	if (ledState.timer >= ledState.functionSpeed) {
		return portMAX_DELAY;
	}

	// Update timer and set block duration (execution time is not counted for simplicity)
	return pdMS_TO_TICKS(1000 / B_LED_UPDATE_HZ);
}

static TickType_t B_RainbowFunction1Renderer()
{
	static int index = 0;
	B_color_t* startColor = &rainbow[index];
	B_color_t* endColor = &rainbow[(index + 1) % (sizeof(rainbow) / sizeof(B_color_t))];

	ledState.timer += (1000 / B_LED_UPDATE_HZ);

	// Save LERPed color into the state, to be able to transition from it when state changes to solo color
	B_ColorLerp(startColor, endColor, (float)ledState.timer / (float)ledState.functionSpeed, &ledState.color);
	B_SetPWMColor(&ledState.color);

	// Restart timer on next color
	if (ledState.timer >= ledState.functionSpeed) {
		ledState.timer = 0;
		index = (index + 1) % (sizeof(rainbow) / sizeof(B_color_t));
	}

	// Update timer and set block duration (execution time is not counted for simplicity)
	return pdMS_TO_TICKS(1000 / B_LED_UPDATE_HZ);
}

static TickType_t B_RainbowFunction2Renderer()
{
	ledState.timer = (ledState.timer + (1000 / B_LED_UPDATE_HZ)) % ledState.functionSpeed;

	// Save current color into the state, to be able to transition from it when state changes to solo color
	ledState.color = B_HSLtoRGB(ledState.timer / (float)ledState.functionSpeed * 360.0f, 1.0f, 0.5f);
	B_SetPWMColor(&ledState.color);

	// Update timer and set block duration (execution time is not counted for simplicity)
	return pdMS_TO_TICKS(1000 / B_LED_UPDATE_HZ);
}

void B_LedControllerTask(void* pvParameters)
{
	const struct B_LedControllerTaskParameter* const taskParameter = (const struct B_LedControllerTaskParameter* const)pvParameters;
	if (taskParameter == NULL || taskParameter->addressMap == NULL) {
		ESP_LOGE(ledControllerTag, "Led Controller task parameter invalid, aborting startup");
		vTaskDelete(NULL);
	}

	B_SetUpPwmChannels();

	TickType_t blockTicks = portMAX_DELAY; // Variably blocks the receive function, used to time trasitions and functions
	B_command_t command = { 0 };
	while (true) {

		// Get new command from command queue if there is one
		QueueHandle_t ledQueue = B_GetAddress(taskParameter->addressMap, B_TASKID_LED);
		if (xQueueReceive(ledQueue, (void* const)&command, blockTicks) == pdPASS) {

			uint8_t commandOP = B_COMMAND_OP(command.header);
			uint8_t commandID = B_COMMAND_ID(command.header);

			ESP_LOGI(ledControllerTag, "Received LED command ID: %u", commandID);

			// Handle received command
			if (commandOP == B_COMMAND_OP_GET) { // --- GET
				B_command_t responseCommand = { 0 };

				responseCommand.header = B_COMMAND_OP_RES | commandID;
				responseCommand.from = B_TASKID_LED;
				responseCommand.dest = command.from;

				switch (commandID) {
					case B_LED_COMMAND_STATE: // STATUS, FUNCTION_ID, FUNCTION_SPEED_HIGH, FUNCTION_SPEED_LOW, RED, GREEN, BLUE
						B_FillCommandBody_BYTE(&responseCommand, 0, ledState.isOn);
						B_FillCommandBody_BYTE(&responseCommand, 1, ledState.functionID);
						B_FillCommandBody_WORD(&responseCommand, 2, ledState.functionSpeed);
						B_FillCommandBody_BYTE(&responseCommand, 4, ledState.color.red);
						B_FillCommandBody_BYTE(&responseCommand, 5, ledState.color.green);
						B_FillCommandBody_BYTE(&responseCommand, 6, ledState.color.blue);
						break;

					default:
						ESP_LOGE(ledControllerTag, "Invalid GET command ID received");
						responseCommand.header = B_COMMAND_OP_ERR;
						B_FillCommandBodyString(&responseCommand, "Invalid GET command ID received");
						break;
				}

				// Send back response
				QueueHandle_t responseQueue = B_GetAddress(taskParameter->addressMap, responseCommand.dest);
				if (xQueueSend(responseQueue, &responseCommand, 0) != pdPASS) {
					ESP_LOGE(ledControllerTag, "Failed to send data back to sender");
				}

			}
			else if (commandOP == B_COMMAND_OP_SET) { // --- SET

				switch (commandID) {
					case B_LED_COMMAND_COLOR: // RED, GREEN, BLUE, TIME_HIGHPART, TIME_LOWPART
						ledState.isOn = true;
						ledState.functionID = B_LED_FUNCTION_NONE;
						ledState.functionSpeed = B_ReadCommandBody_WORD(&command, 3);

						ledState.previousColor = ledState.color;
						ledState.color.red = B_ReadCommandBody_BYTE(&command, 0);
						ledState.color.green = B_ReadCommandBody_BYTE(&command, 1);
						ledState.color.blue = B_ReadCommandBody_BYTE(&command, 2);
						ledState.timer = 0;

						B_SendStatusReply(taskParameter->addressMap, B_TASKID_LED, command.from, B_COMMAND_OP_RES, commandID, "OK");
						break;

					case B_LED_COMMAND_FUNCTION: // FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART
						// Sanitize function id
						uint8_t newFunctionID = B_ReadCommandBody_BYTE(&command, 0);
						if (newFunctionID >= B_LED_FUNCTION_ENUM_SIZE || newFunctionID == B_LED_FUNCTION_NONE) {
							ESP_LOGW(ledControllerTag, "Invalid function ID received");
							B_SendStatusReply(taskParameter->addressMap, B_TASKID_LED, command.from, B_COMMAND_OP_ERR, commandID, "Invalid function ID");
							break;
						}

						ledState.isOn = true;
						ledState.functionID = newFunctionID;
						ledState.functionSpeed = B_ReadCommandBody_WORD(&command, 1);
						ledState.timer = 0;

						B_SendStatusReply(taskParameter->addressMap, B_TASKID_LED, command.from, B_COMMAND_OP_RES, commandID, "OK");
						break;

					default:
						ESP_LOGW(ledControllerTag, "Invalid SET command ID received");
						B_SendStatusReply(taskParameter->addressMap, B_TASKID_LED, command.from, B_COMMAND_OP_ERR, commandID, "Invalid SET command");
						break;
				}
			}

		}

		// TODO: The speed for function 2 is for the entire rainbow, while for function 1 it is for one transition, making the second much faster compared to the first one
		// Handle LED state
		switch (ledState.functionID) {
			case B_LED_FUNCTION_NONE:
				blockTicks = B_ColorTransitionRenderer();
				break;

			case B_LED_FUNCTION_RAINBOW1:
				blockTicks = B_RainbowFunction1Renderer();
				break;

			case B_LED_FUNCTION_RAINBOW2:
				blockTicks = B_RainbowFunction2Renderer();
				break;

			default:
				ESP_LOGE(ledControllerTag, "Provided function id not found: %u", ledState.functionID);
				blockTicks = portMAX_DELAY;
				break;
		}
	}

	// Task paniced, clean up and delete task
	vTaskDelete(NULL);
}

void B_SetUpGpioPin(gpio_num_t pin)
{
	ESP_ERROR_CHECK(gpio_reset_pin(pin));
	ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_DEF_OUTPUT));
	ESP_LOGI(ledControllerTag, "GPIO is on for pin: %i", pin);
}

void B_SetGpioMode(gpio_num_t pin, bool mode)
{
	ESP_ERROR_CHECK(gpio_set_level(pin, mode));
}

void B_GPIORotateColorExample()
{
	B_SetUpGpioPin(CONFIG_B_RED_PIN);
	B_SetUpGpioPin(CONFIG_B_GREEN_PIN);
	B_SetUpGpioPin(CONFIG_B_BLUE_PIN);

	int counter = 0;
	while (true) {
		B_SetGpioMode(CONFIG_B_RED_PIN, counter == 0);
		B_SetGpioMode(CONFIG_B_GREEN_PIN, counter == 1);
		B_SetGpioMode(CONFIG_B_BLUE_PIN, counter == 2);
		counter = (counter + 1) % 3;

		ESP_LOGI(ledControllerTag, "PING %i", counter);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
