#ifndef __PROTOCOL_HANDLER_H__
#define __PROTOCOL_HANDLER_H__

#include "byte.h"
#include "config.h"

typedef enum
	{ NOP = 0x00
	, STOP = 0x01
	, MOVE = 0x02
	, READ_VALUE = 0x03
	, NATIVE_CODE = 0x04
	, GET_ENTRY_POINT = 0x05
	, GET_FUNC_TABLE = 0x06
	} ProtoCommandType;

typedef enum
	{ DONE = 0x00
	, VALUE = 0x01
	, ENTRY_POINT = 0x02
	, FUNC_TABLE = 0x03
	} ProtoCommandResponseType;

typedef enum
	{ STRAIGHT = 0x00
	, TURN_LEFT = 0x01
	, TURN_RIGHT = 0x02
	} MoveType;

typedef enum
	{ NO_OP = 0xff
	, BFALSE = 0x00
	, BTRUE = 0x01
	, NOT = 0x02
	, AND = 0x03
	, OR = 0x04
	, GT = 0x05
	, EQ = 0x06
	, TIME_LIMIT = 0x07
	} CondOp;

typedef enum
	{ NO_TYPE = 0xff
	, CONST_INT = 0x00
	, SENSOR = 0x01
	} CondAtomType;

// initializer (for the interrupt)
void proto_init();

void proto_fake();

// input interrupt handler
void proto_read();
void proto_send(byte len);

// helper functions
bit proto_cmd_ready(); // Is there a command ready to parse?
int proto_read_sensor(byte sensor); // Reads a sensor, translates also "virtual sensors" into "real" ones
//void proto_dump_message(byte *buf, byte len); // Dump a message to LCD display
void proto_wait_condition();
void proto_send_done();
//byte proto_read_position();
//byte proto_read_position_2();


// parser
byte proto_evaluate_cond(byte xdata **buf) ;
void proto_execute();


#endif

