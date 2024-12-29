#include "B_BarnaNetCommand.h"

#define B_COMMAND_DEST_LED (uint8_t) 1 // Should not add a DEST with a value of 0, as that may be interpreted as a set command of that type
#define B_COMMAND_DEST_ALARM (uint8_t) 2
