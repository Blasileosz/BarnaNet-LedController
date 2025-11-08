from library import *
from enum import Enum

class LedControllerService(Service):
	B_COMMAND_DEST_LED_CONTROLLER = 11

	B_LED_COMMAND_STATE = 0 # get / set
	B_LED_COMMAND_COLOR = 1 # Only set
	B_LED_COMMAND_FUNCTION = 2 # Only set

	class Functions(Enum):
		B_LED_FUNCTION_RAINBOW1 = 1
		B_LED_FUNCTION_RAINBOW2 = 2

	# Experimental
	@staticmethod
	def Build_SET_State(is_on: bool, speed: int) -> Command:
		"""
		EXPERIMENTAL
		Set the LED controller state (on/off) and speed for functions
			- speed: 16bit value in milliseconds
		"""
		cmd = Command()
		cmd.SetDest(LedControllerService.B_COMMAND_DEST_LED_CONTROLLER)
		cmd.SetHeader(B_COMMAND_OP_SET, LedControllerService.B_LED_COMMAND_STATE)
		cmd.SetBodyByte(0, int(is_on))
		cmd.SetBodyWord(1, speed)
		return cmd

	@staticmethod
	def Build_GET_State() -> Command:
		"""
		Get the LED controller state (on/off), function ID, function speed, and RGB color
		"""
		cmd = Command()
		cmd.SetDest(LedControllerService.B_COMMAND_DEST_LED_CONTROLLER)
		cmd.SetHeader(B_COMMAND_OP_GET, LedControllerService.B_LED_COMMAND_STATE)
		return cmd

	@staticmethod
	def Parse_RES_GET_State(command: Command) -> str:
		body = command.GetBodyBytes()
		state = {
			"is_on": bool(body[0]),
			"functionID": body[1],
			"functionSpeed": DeserializeWORD(body[2:4]),
			"red": body[4],
			"green": body[5],
			"blue": body[6]
		}
		return str(state)

	@staticmethod
	def Build_SET_Color(red: int, green: int, blue: int, speed: int) -> Command:
		"""
		Set the LED color and speed
			- color values: 0-255
			- speed: 16bit value in milliseconds
		"""
		cmd = Command()
		cmd.SetDest(LedControllerService.B_COMMAND_DEST_LED_CONTROLLER)
		cmd.SetHeader(B_COMMAND_OP_SET, LedControllerService.B_LED_COMMAND_COLOR)
		cmd.SetBodyByte(0, red)
		cmd.SetBodyByte(1, green)
		cmd.SetBodyByte(2, blue)
		cmd.SetBodyWord(3, speed)
		return cmd

	@staticmethod
	def Build_SET_Function(function: int, speed: int) -> Command:
		"""
		Set the LED function and speed
			- function: LED function ID
				- Rainbow1: 1
				- Rainbow2: 2
			- speed: 16bit value in milliseconds
		"""
		if function not in [func.value for func in LedControllerService.Functions]:
			raise ValueError("Invalid function ID")

		cmd = Command()
		cmd.SetDest(LedControllerService.B_COMMAND_DEST_LED_CONTROLLER)
		cmd.SetHeader(B_COMMAND_OP_SET, LedControllerService.B_LED_COMMAND_FUNCTION)
		cmd.SetBodyByte(0, function)
		cmd.SetBodyWord(1, speed)
		return cmd
