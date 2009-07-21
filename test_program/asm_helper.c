#include "asm_helper.h"
#include <stdio.h>
#include "lcd.h"
#include "driver.h"
#include "sharp_sensor.h"

byte xdata *asm_funcTable[FUNCTION_LAST];

void asm_call(byte xdata *addr) {
	addr = addr;

	// push the address onto the stack, so that this function's RET calls it
#pragma asm
	MOV A, R7
	PUSH ACC
	MOV A, R6
	PUSH ACC
#pragma endasm
}

void asm_initFuncTable() {
	asm_funcTable[FUNCTION_STOP]          = (byte xdata *) driver_stop;
	asm_funcTable[FUNCTION_PAUSE]         = (byte xdata *) driver_pause;
	asm_funcTable[FUNCTION_UNPAUSE]       = (byte xdata *) driver_unpause;
	asm_funcTable[FUNCTION_MOVEFORWARD]   = (byte xdata *) driver_moveForward;
	asm_funcTable[FUNCTION_MOVEBACKWARD]  = (byte xdata *) driver_moveBackward;
	asm_funcTable[FUNCTION_ROTATELEFT]    = (byte xdata *) driver_rotateLeft;
	asm_funcTable[FUNCTION_ROTATERIGHT]   = (byte xdata *) driver_rotateRight;
	asm_funcTable[FUNCTION_DRIVERSTATUS]  = (byte xdata *) driver_status;
	asm_funcTable[FUNCTION_DRIVE]         = (byte xdata *) driver_drive;
	asm_funcTable[FUNCTION_DRIVESTRAIGHT] = (byte xdata *) driver_driveStraight;
	asm_funcTable[FUNCTION_ROTATE]        = (byte xdata *) driver_rotate;
	asm_funcTable[FUNCTION_GETSENSOR]     = (byte xdata *) sharpa_measure;
	asm_funcTable[FUNCTION_SENSORTOCM]    = (byte xdata *) samplea_to_cm;
}

byte xdata *asm_getFuncAddress(Function func) {
	func %= FUNCTION_LAST;

	return asm_funcTable[func];
}

byte xdata **asm_getFuncTable() {
	return asm_funcTable;
}