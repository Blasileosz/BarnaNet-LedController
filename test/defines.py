
HOST = '192.168.0.80'
PORT = 2005

SerializeDWORD = lambda n : [int(i) for i in n.to_bytes(4, byteorder='big', signed=True)]
SerializeWORD = lambda n : [int(i) for i in n.to_bytes(2, byteorder='big', signed=True)]

B_COMMAND_OP_MASK = 0b11000000
B_COMMAND_OP_SET = 0b00000000
B_COMMAND_OP_GET = 0b01000000
B_COMMAND_OP_RES = 0b10000000
B_COMMAND_OP_ERR = 0b11000000

B_COMMAND_ID_MASK = 0b00111111

B_PLACEHOLDER_FROM = 0 # No need to send the from field in the tcp message
B_PLACEHOLDER_TID = 0 # No need to send the transmission ID field in the tcp message

B_COMMAND_DEST_TCP = 1
B_COMMAND_DEST_ALARM = 2
B_COMMAND_DEST_MQTT = 3
B_COMMAND_DEST_LED = 11

B_LED_COMMAND_STATE = 0 # get/set
B_LED_COMMAND_COLOR = 1 # Only set
B_LED_COMMAND_FUNCTION = 2 # Only set

B_ALARM_COMMAND_INSERT = 1 # Only set
B_ALARM_COMMAND_REMOVE = 2 # Only set
B_ALARM_COMMAND_LIST = 3 # Only get
B_ALARM_COMMAND_INSPECT = 4 # Only get

B_MONDAY = 0b00000010
B_TUESDAY = 0b00000100
B_WEDNESDAY = 0b00001000
B_THURSDAY = 0b00010000
B_FRIDAY = 0b00100000
B_SATURDAY = 0b01000000
B_SUNDAY = 0b00000001
B_WEEKDAYS = 0b00111110
B_WEEKENDS = 0b01000001
B_EVERYDAY = 0b01111111

def GetLocalTimepart():
	from datetime import datetime, timezone, timedelta

	# adjust for docker tz
	td = timedelta(hours=2)
	tz = timezone(td)

	now = datetime.now(tz)
	return now.second + now.minute * 60 + now.hour * 3600

def GetTimepart(hours, minutes, seconds):
	return seconds + minutes * 60 + hours * 3600
