#include "B_BarnaNetCommand.h"

// Should not add a DEST with a value of 0, as an empty command may be interpreted as a SET command of that type
// DEST value of 1 and 2 are dedicated to the tcp and alarm system
enum B_COMMAND_DEST_ENUM_LED {
	B_COMMAND_DEST_LED = 3,
};
