#ifndef __ASM_HELPER_H__
#define __ASM_HELPER_H__

#ifndef __byte__
#define __byte__
typedef unsigned char byte;
#endif

typedef enum {
	FUNCTION_STOP = 0,
	FUNCTION_PAUSE,
	FUNCTION_UNPAUSE,
	FUNCTION_MOVEFORWARD,
	FUNCTION_MOVEBACKWARD,
	FUNCTION_ROTATELEFT,
	FUNCTION_ROTATERIGHT,

	FUNCTION_DRIVERSTATUS,

	FUNCTION_DRIVE,
	FUNCTION_DRIVESTRAIGHT,
	FUNCTION_ROTATE,

	FUNCTION_GETSENSOR,
	FUNCTION_SENSORTOCM,

	FUNCTION_LAST
} Function;

void asm_call(byte xdata *addr);

void asm_initFuncTable();
byte xdata *asm_getFuncAddress(Function func);
byte xdata **asm_getFuncTable();

#endif // __ASM_HELPER_H__
