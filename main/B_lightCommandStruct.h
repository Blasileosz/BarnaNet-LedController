#pragma once

#include <stdio.h>

#define B_CHANGECOLOR_COMMAND 0b0
/* Expected data
	bytes 0-2 -> r,g,b
	bytes 3-4 -> lerp time (ms)
*/

typedef struct {
	// i = Command id
	// s = strip id
	// 00000000 ssiiiiii
	uint16_t header;

	uint8_t data[16];
} B_command_t;
