#pragma once

#include <stdio.h>
#include "B_colorUtil.h"
#include "B_globalState.h"

#define B_COMMAND_MAX_LEN 128


typedef struct {
	uint8_t commandID;
	uint8_t stripID; // TODO: add feature

	uint8_t data[126];
} B_command_t;


#define B_SET_COMMAND_MASK 0b00000000
#define B_GET_COMMAND_MASK 0b01000000
#define B_RESPONSE_COMMAND_MASK 0b10000000
#define B_ERROR_COMMAND_MASK 0b11000000

#define B_COLOR_COMMAND 0
#define B_FUNCION_COMMAND 1

// -- SET -- //

// SET COLOR - Expected data
// - bytes 0-2: r,g,b
// - bytes 3-4: lerp time (ms)
// - Example: [B_COMMAND_SETCOLOR, unused, RED, GREEN, BLUE, TIME_HIGHPART, TIME_LOWPART]
#define B_COMMAND_SETCOLOR B_SET_COMMAND_MASK | B_COLOR_COMMAND

// SET FUNCTION - Expected data
// - bytes 0: function id
// - bytes 1-2: speed (ms)
// - Example: [B_COMMAND_SETFUNCTION, unused, FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART]
#define B_COMMAND_SETFUNCTION B_SET_COMMAND_MASK | B_FUNCION_COMMAND

// -- GET & RESPONSE -- //

// GET COLOR - Expected data: none
// - Example: [B_COMMAND_GETCOLOR, unused]
#define B_COMMAND_GETCOLOR B_GET_COMMAND_MASK | B_COLOR_COMMAND

// RESPOND COLOR - Response
// - bytes 0-2: r,g,b
// - Example: [B_COMMAND_RESPONDCOLOR, unused, RED, GREEN, BLUE]
#define B_COMMAND_RESPONDCOLOR (B_RESPONSE_COMMAND_MASK | B_COLOR_COMMAND)


// GET FUNCTION - Expected data: none
// - Example: [B_COMMAND_GETFUNCTION, unused]
#define B_COMMAND_GETFUNCTION B_GET_COMMAND_MASK | B_FUNCION_COMMAND

// RESPOND FUNCTION - Response
// - bytes 0: function id
// - bytes 1-2: function speed
// - Example: [B_COMMAND_RESPONDFUNCTION, unused, FUNCTION_ID, SPEED_HIGHPART, SPEED_LOWPART]
#define B_COMMAND_RESPONDFUNCTION B_RESPONSE_COMMAND_MASK | B_FUNCION_COMMAND
