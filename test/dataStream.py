import json
from defines import *

data = [0, B_COMMAND_DEST_LED, B_COMMAND_OP_SET | B_LED_COMMAND_FUNCTION, 2, 0b10000000, 0b00000000]
print(data)
print(json.dumps(data))
