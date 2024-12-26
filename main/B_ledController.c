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


static void B_SetUpPwmChanels()
{
	ledc_timer_config_t timer = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_num = LEDC_TIMER_0,
		.duty_resolution = LEDC_TIMER_8_BIT,
		.freq_hz = 5000,
		.clk_cfg = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&timer));

	ledc_channel_config_t redChanel = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_0,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = RED_PIN,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&redChanel));

	ledc_channel_config_t greenChannel = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_1,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = GREEN_PIN,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&greenChannel));

	ledc_channel_config_t blueChannel = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = LEDC_CHANNEL_2,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = BLUE_PIN,
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
	return (1000 / B_LED_UPDATE_HZ) * portTICK_PERIOD_MS;
}

static TickType_t B_RainbowFunction1Renderer()
{
	static int index = 0;
	B_color_t* startColor = &rainbow[index];
	B_color_t* endColor = &rainbow[(index + 1) % (sizeof(rainbow) / sizeof(B_color_t))];

	ledState.timer += (1000 / B_LED_UPDATE_HZ);

	B_color_t lerpedColor;
	B_ColorLerp(startColor, endColor, (float)ledState.timer / (float)ledState.functionSpeed, &lerpedColor);

	B_SetPWMColor(&lerpedColor);

	// Restart timer on next color
	if (ledState.timer >= ledState.functionSpeed) {
		ledState.timer = 0;
		index = (index + 1) % (sizeof(rainbow) / sizeof(B_color_t));
	}

	// Update timer and set block duration (execution time is not counted for simplicity)
	return (1000 / B_LED_UPDATE_HZ) * portTICK_PERIOD_MS;
}

static TickType_t B_RainbowFunction2Renderer()
{
	ledState.timer = (ledState.timer + (1000 / B_LED_UPDATE_HZ)) % ledState.functionSpeed;
	B_color_t color = B_HSLtoRGB(ledState.timer / (float)ledState.functionSpeed * 360.0f, 1.0f, 0.5f);

	B_SetPWMColor(&color);

	// Update timer and set block duration (execution time is not counted for simplicity)
	return (1000 / B_LED_UPDATE_HZ) * portTICK_PERIOD_MS;
}

void B_LedControllerTask(void* pvParameters)
{
	const B_LedControllerTaskParameter_t* const queues = (const B_LedControllerTaskParameter_t* const)pvParameters;
	if (queues == NULL || queues->tcpCommandQueue == NULL || queues->ledCommandQueue == NULL) {
		ESP_LOGE(ledControllerTag, "Led Controller task parameter invalid, aborting startup");
		vTaskDelete(NULL);
	}

	B_SetUpPwmChanels();

	TickType_t blockTicks = portMAX_DELAY; // Variably blocks the receive function, also used to time trasitions and functions
	B_command_t command = { 0 };
	while (true) {
		// Get new command from command queue if there is one
		if (xQueueReceive(*queues->ledCommandQueue, (void* const)&command, blockTicks) == pdPASS) {
			ESP_LOGI(ledControllerTag, "Received LED command: header: %u, ID: %u", command.header, command.ID);

			// Handle received command
			if (B_COMMAND_OP(command.header) == B_COMMAND_OP_GET) { // --- GET
				B_command_t responseCommand = { 0 };

				switch (command.ID) {
					case B_LED_COMMAND_STATE: // STATUS, FUNCTION_ID, FUNCTION_SPEED_HIGH, FUNCTION_SPEED_LOW, RED, GREEN, BLUE
						responseCommand.header = B_COMMAND_OP_RES | B_COMMAND_DEST(command.header);
						responseCommand.ID = command.ID;
						responseCommand.data[0] = ledState.isOn;
						responseCommand.data[1] = ledState.functionID;
						*((uint16_t*)&responseCommand.data[2]) = ntohs(ledState.functionSpeed);
						responseCommand.data[4] = ledState.color.red;
						responseCommand.data[5] = ledState.color.green;
						responseCommand.data[6] = ledState.color.blue;
						break;

					default:
						ESP_LOGE(ledControllerTag, "Invalid GET command ID received");
						responseCommand.header = B_COMMAND_OP_ERR | B_COMMAND_DEST(command.header);
						responseCommand.ID = command.ID;
						responseCommand.data[0] = B_COMMAND_ERR_CLIENT;
						break;
				}

				// Send back response
				if (xQueueSend(*queues->tcpCommandQueue, &responseCommand, 0) != pdPASS) {
					ESP_LOGE(ledControllerTag, "Failed to send data back to TCP server");
				}

			}
			else if (B_COMMAND_OP(command.header) == B_COMMAND_OP_SET) { // --- SET

				switch (command.ID) {
					case B_LED_COMMAND_COLOR: // RED, GREEN, BLUE, TIME_HIGHPART, TIME_LOWPART
						ledState.isOn = true;
						ledState.functionID = B_LED_FUNCTION_NONE;
						ledState.functionSpeed = htons(*((uint16_t*)(&command.data[3])));
						ledState.previousColor = ledState.color;
						ledState.color.red = command.data[0];
						ledState.color.green = command.data[1];
						ledState.color.blue = command.data[2];
						ledState.timer = 0;
						break;

					case B_LED_COMMAND_FUNCION: // FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART
						ledState.isOn = true;
						ledState.functionID = B_LED_FUNCTION_RAINBOW1;
						ledState.functionSpeed = htons(*((uint16_t*)(&command.data[1])));
						ledState.timer = 0;
						break;

					default:
						ESP_LOGE(ledControllerTag, "Invalid SET command ID received");
						// TODO: Respond?
						break;
				}
			}

		}

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
	gpio_reset_pin(pin);
	ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_DEF_OUTPUT));
	ESP_LOGI(ledControllerTag, "GPIO is on for pin: %i", pin);
}

void B_SetGpioMode(gpio_num_t pin, bool mode)
{
	ESP_ERROR_CHECK(gpio_set_level(pin, mode));
}

void B_GPIORotateColorExample()
{
	B_SetUpGpioPin(RED_PIN);
	B_SetUpGpioPin(GREEN_PIN);
	B_SetUpGpioPin(BLUE_PIN);

	int counter = 0;
	while (true) {
		B_SetGpioMode(RED_PIN, counter == 0);
		B_SetGpioMode(GREEN_PIN, counter == 1);
		B_SetGpioMode(BLUE_PIN, counter == 2);
		counter = (counter + 1) % 3;

		ESP_LOGI(ledControllerTag, "PING %i", counter);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
