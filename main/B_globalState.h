#pragma once

#include <stdio.h>
#include "B_colorUtil.h"
#include "B_protocolCommands.h"

typedef struct {
	bool isOn;
	B_color_t color;
	int functionID;
	int functionSpeed;
} B_globalState_t;


//void B_STATE_setColor(B_command_t command, B_globalState_t globalSate, SemaphoreHandle_t mutex);
