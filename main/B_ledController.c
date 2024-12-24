#include "B_ledController.h"

static const char* ledControllerTag = "BarnaNet - LedController";

void B_SetUpPwmChanels()
{
	ledc_timer_config_t timer = {
	    .speed_mode = LEDC_HIGH_SPEED_MODE,
	    .timer_num = LEDC_TIMER_0,
	    .duty_resolution = LEDC_TIMER_8_BIT,
	    .freq_hz = 5000,
	    .clk_cfg = LEDC_AUTO_CLK};
	ESP_ERROR_CHECK(ledc_timer_config(&timer));

	ledc_channel_config_t redChanel = {
	    .speed_mode = LEDC_HIGH_SPEED_MODE,
	    .channel = LEDC_CHANNEL_0,
	    .timer_sel = LEDC_TIMER_0,
	    .intr_type = LEDC_INTR_DISABLE,
	    .gpio_num = RED_PIN,
	    .duty = 0,
	    .hpoint = 0};
	ESP_ERROR_CHECK(ledc_channel_config(&redChanel));

	ledc_channel_config_t greenChannel = {
	    .speed_mode = LEDC_HIGH_SPEED_MODE,
	    .channel = LEDC_CHANNEL_1,
	    .timer_sel = LEDC_TIMER_0,
	    .intr_type = LEDC_INTR_DISABLE,
	    .gpio_num = GREEN_PIN,
	    .duty = 0,
	    .hpoint = 0};
	ESP_ERROR_CHECK(ledc_channel_config(&greenChannel));

	ledc_channel_config_t blueChannel = {
	    .speed_mode = LEDC_HIGH_SPEED_MODE,
	    .channel = LEDC_CHANNEL_2,
	    .timer_sel = LEDC_TIMER_0,
	    .intr_type = LEDC_INTR_DISABLE,
	    .gpio_num = BLUE_PIN,
	    .duty = 0,
	    .hpoint = 0};
	ESP_ERROR_CHECK(ledc_channel_config(&blueChannel));

	ESP_LOGI(ledControllerTag, "Configured PWM module");
}

void B_SetPWMColor(B_color_t* color) {
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, color->red));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0));

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, color->green));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1));

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, color->blue));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2));
}

void B_LedControllerTask(void* pvParameters)
{
	B_SetUpPwmChanels();
	
	//QueueHandle_t* commandQueuePtr = (QueueHandle_t*)pvParameters;

	B_command_t commandBuffer = {0};
	B_commandExecution_t commandExecution = {0};
	while (true)
	{
		// Get new command from command queue if there is one
		// if (xQueueReceive(*commandQueuePtr, &commandBuffer, 0) == pdPASS)
		// {
		// 	commandExecution.state = LED_STATE_INPROGRESS;
		// 	commandExecution.timer = 0;
		// };

		// Handle the command based on its ID
		switch (commandBuffer.commandID)
		{
			case B_COMMAND_SETCOLOR:
			{
				if (commandExecution.state != LED_STATE_INPROGRESS)
					break;
				
				// Update color if state is in progress
				int totalTransitionTime = htons(*((uint16_t*)(&commandBuffer.data[3])));
				B_color_t* desiredColor = (B_color_t*)(&commandBuffer.data[0]);
				B_color_t lerpedColor;
				B_ColorLerp(&commandExecution.previousColor, desiredColor, (float)commandExecution.timer / (float)totalTransitionTime, &lerpedColor);

				B_SetPWMColor(&lerpedColor);

				if (commandExecution.timer >= totalTransitionTime){
					commandExecution.state = LED_STATE_FINISHED;
					commandExecution.previousColor = *desiredColor;
				}
				break;
			}
		
			default:
				break;
		}

		// Update timer with delay (execution time is not counted for simplicity)
		if (commandExecution.state == LED_STATE_INPROGRESS)
			commandExecution.timer += (1000 / LED_UPDATE_HZ);

		vTaskDelay((1000 / LED_UPDATE_HZ) / portTICK_PERIOD_MS);
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
	while (true)
	{
		B_SetGpioMode(RED_PIN, counter == 0);
		B_SetGpioMode(GREEN_PIN, counter == 1);
		B_SetGpioMode(BLUE_PIN, counter == 2);
		counter = (counter + 1) % 3;

		ESP_LOGI(ledControllerTag, "PING %i", counter);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
