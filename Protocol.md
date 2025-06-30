# BarnaNet-LedController API endpoints

## LED-Controller task
- Task id = 3
- LED functions
	- `B_LED_FUNCTION_RAINBOW1 = 1` - Experimental rainbow
	- `B_LED_FUNCTION_RAINBOW2 = 2`


### STATE
- OP: Get
- ID: 0
- Request layout: `No data required`
- Response layout: `[IS_ON, FUNCTION_ID, FUNCTION_SPEED_HIGH, FUNCTION_SPEED_LOW, RED, GREEN, BLUE]`
	- IS_ON: A boolean value indicating if the light is on
	- FUNCTION_ID: The id of the function currently selected (the value is 0 if the light is a static color)
	- FUNCTION_SPEED: An unsigned 16bit millisecond value, showing how fast the function is animating (when the color is a solid color it meant the transition time)
	- RED, GREEN, BLUE: Unsigned 8bit color fields

### COLOR
- OP: Set
- ID: 1
- Request layout: `[RED, GREEN, BLUE, SPEED_HIGHPART, SPEED_LOWPART]`
	- RED, GREEN, BLUE: Unsigned 8bit color values
	- SPEED: Unsigned 16bit transition time in milliseconds
- Response layout: A plaintext status

### FUNCTION
- OP: Set
- ID: 2
- Request layout: `[FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART]`
	- FUNCTION_ID: The id of the function
	- SPEED: An unsigned 16bit millisecond value, showing how fast the function is animating
- Response layout: A plaintext status
