#include <REG552.H>               // special function register declarations
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "protocol_handler.h"
#include "driver.h"
#include "lcd.h"
#include "timing.h"
#include "asm_helper.h"
#include "servo.h"
#include "sharp_sensor.h"

byte sensor_last = 0;
// command buffer
byte xdata cmdSizeRec = 0;
bit cmdReady = 0;
unsigned int xdata cmdLen = 0;
unsigned int xdata cmdBufferPos = 0;
unsigned int xdata cmdSendBufferLen = 0;
unsigned int xdata cmdSendBufferPos = 0;
byte xdata cmdBuffer[PROTO_RECV_BUFFER_SIZE];
byte xdata *cmdCond;
byte xdata cmdSendBuffer[PROTO_SEND_BUFFER_SIZE];
TimerStatus xdata condTimerStatus = TIMER_NOP;

struct condVars {
	byte op;
	byte l, r;
	int valueL, valueR;
};

struct condVars condStack[PROTO_MAX_RECURSION_DEPTH];
signed char condStackPos = -1;

void proto_fake() {
	cmdLen = 5;	
	cmdBuffer[0] = 2;
	cmdBuffer[1] = 40;
	cmdBuffer[2] = 2;
	cmdBuffer[3] = 50;
	cmdBuffer[4] = 1;
} 

void proto_init() {
	// Enable interrupt 4 (Serial Port)

	EA = 1;

	TI = 0;				/* clear transmit interrupt */
	RI = 0;				/* clear receiver interrupt */

	ES0 = 1;			/* enable serial interrupts */
	PS0 = 0;			/* set serial interrupts to low priority */
}

void proto_interrupt() interrupt 4 {
	byte rec = 0;	// we use rec to assure that receiving and sending is done as quickly as possible
					// then we handle the received byte
	byte ch;

	if (RI != 0) { // did we receive anything?
		RI = 0;
		ch = S0BUF; // the received character
		rec = 1;
	}
	
	if (TI != 0) { // can we transmit anything?
		TI = 0;

		if (cmdSendBufferPos < cmdSendBufferLen)
			S0BUF = cmdSendBuffer[cmdSendBufferPos++];
	}

	if (!rec)
		return; // nothing received => leave

	if (cmdSizeRec == 0) {
		cmdLen = ch << 8;
		cmdSizeRec = 1;
	} else if (cmdSizeRec == 1) {
		cmdLen += ch;
		cmdSizeRec = 2;
	} else {
		cmdBuffer[cmdBufferPos] = ch;
		cmdBufferPos++;
		if(cmdBufferPos == cmdLen) { // command in buffer is complete
			cmdSizeRec = 0;		
			cmdReady = 1; // signal a ready command
		}
	}
}

void proto_send(byte len) {
	EA = 0;

	cmdSendBufferPos = 0;
	cmdSendBufferLen = len;

	EA = 1;

	S0BUF = cmdSendBuffer[cmdSendBufferPos++];
}

bit proto_cmd_ready() {
	if (cmdReady) {
		return 1;
	}
	else {
		return 0;
	}
}

int proto_read_sensor(byte sensor) {
	unsigned int rs;
	byte i;
//	char xdata dbuf[20];
	if(sensor == 11) {
		return driver_get_pos();
	}
	sensor %= 11;
	if(sensor <= 5) {
		if((sensor_last % 3) != (sensor % 3)) {
			servo_set_pos(30 + (sensor % 3) * 60);
			wait_ms(1000);
		}
		sensor_last = sensor;
		sensor = (sensor <= 2) ? 0 : 1;
	}
	else {
		sensor_last = sensor;
		sensor -= 4;
	}
	rs = sharpa_measure(sensor);
	for(i = 0; i < 4; i++) {
		wait_ms(50);
		rs = (rs + sharpa_measure(sensor)) / 2;
	}
//	sprintf(dbuf, "%u: %u", (unsigned int) (sensor), (unsigned int) (samplea_to_cm(rs, (sensor <= 2 ? SENSOR_LONG_DIST : SENSOR_SHORT_DIST))));
//	lcd_print_string(dbuf);
	return ((int) (samplea_to_cm(rs, (sensor <= 2 ? SENSOR_LONG_DIST : SENSOR_SHORT_DIST))));
}

void proto_wait_condition() {
	byte xdata *cmdCondPointer = cmdCond;
	while(proto_evaluate_cond(&cmdCondPointer)) {
		//wait_ms(50);
		driver_save_pos(); // save driver postition for next evaluation
		cmdCondPointer = cmdCond;
	}
}

/*void proto_dump_message(byte *buf, byte len) {
	char xdata sbuf[40];
	byte i;
	lcd_clear();
	strcpy(sbuf, "Message:            ");
	lcd_write_text(sbuf);
	for(i = 0; i < len; i++) {
		sprintf(sbuf, "%u ", (unsigned int) *(buf + i));
		lcd_write_text(sbuf);
	}
}*/

void proto_send_done() {
	cmdSendBuffer[0] = DONE;
	proto_send(1);
}

byte proto_evaluate_cond(byte xdata **buf) {
	if(condStackPos == (PROTO_MAX_RECURSION_DEPTH - 1)) {
		lcd_print_string("Recursion depth exceeded!");
		while(1);		
	}

	condStackPos++;

	condStack[condStackPos].op = 0;
	if(buf == 0 || *buf == 0) {
		return 0;
	}
	condStack[condStackPos].op = **buf;
	(*buf)++;
	switch(condStack[condStackPos].op) {
		case BFALSE:			
			condStackPos--;
			return 0;
		case BTRUE:			
			condStackPos--;
			return 1;
		case NOT:			
			condStackPos--;
			return !proto_evaluate_cond(buf);
		case AND:
		case OR:			
			condStack[condStackPos].l = 0;
			condStack[condStackPos].r = 0;
			condStack[condStackPos].l = proto_evaluate_cond(buf);
			condStack[condStackPos].r = proto_evaluate_cond(buf);
			condStackPos--;
			if(condStack[condStackPos + 1].op==AND) {
				return (condStack[condStackPos + 1].l && condStack[condStackPos + 1].r);
			} else {
				return (condStack[condStackPos + 1].l || condStack[condStackPos + 1].r);
			}				
			break;
		case GT:
		case EQ:			
			condStack[condStackPos].valueL = 0;
			condStack[condStackPos].valueR = 0;
			switch(**buf) {
				case CONST_INT:
					(*buf)++;
					condStack[condStackPos].valueL = (int) ((**buf)<<8);
					(*buf)++;
					condStack[condStackPos].valueL += (int) (**buf);
					(*buf)++;
					break;
				case SENSOR:
					(*buf)++;
					condStack[condStackPos].valueL = proto_read_sensor(**buf);
					(*buf)++;
					break;
				default:
					lcd_print_string("NUM ERROR_1!");
					break;
			}
			switch(**buf) {
				case CONST_INT:
					(*buf)++;
					condStack[condStackPos].valueR = (int) ((**buf)<<8);
					(*buf)++;
					condStack[condStackPos].valueR += (int) (**buf);
					(*buf)++;
					break;
				case SENSOR:
					(*buf)++;
					condStack[condStackPos].valueR = proto_read_sensor(**buf);
					(*buf)++;
					break;
				default:
					lcd_print_string("NUM ERROR_2!");
					break;
			}
			switch(condStack[condStackPos].op) {
				case GT:
					condStackPos--;
					return (condStack[condStackPos+1].valueL > condStack[condStackPos+1].valueR);
					break;
				case EQ:					
					condStackPos--;
					return (condStack[condStackPos+1].valueL == condStack[condStackPos+1].valueR);
					break;
			}			
			break;
		case TIME_LIMIT:
			{
				unsigned int timeLimit = 0;
				timeLimit = (**buf)<<8;
				(*buf)++;
				timeLimit += **buf;
				(*buf)++;
				switch(condTimerStatus) {
					case TIMER_NOP: 					
						wait_with_timer(timeLimit);
						condStackPos--;
						return 1;
						break;
					case TIMER_STARTED:
							condStackPos--;
						return 1;
						break;
					case TIMER_REACHED:
						condStackPos--;
						return 0;
						break;
					default:
						lcd_print_string("TIMER ERROR!");
						break;
				}
			}
			break;
		default:			
			lcd_print_string("OP ERROR!");
			break;
	}
	condStackPos--;
	return 0;
}

void proto_execute() {
	byte pos = 0;
	char speed = 0;
	byte moveType = 0, dir = 0, func = 0;
	DriverStatus *drvStatus = 0;
	byte xdata *cmdCondPointer;
	if(cmdBuffer == 0) {
		return;
	}

	condTimerStatus = TIMER_NOP;
	cmdReady = 0; // the command isn't ready anymore...
	switch(cmdBuffer[pos]) {
		case NOP:			
			lcd_print_string("NOP");
			*cmdCond = NO_OP;
			proto_send_done();
			break;
		case MOVE:
			pos++;
			speed = (char) (cmdBuffer[pos]);
			pos++;
			moveType = cmdBuffer[pos];
			pos++;
			switch(moveType) {
				case STRAIGHT:
					lcd_print_string("DRIVE_STRAIGHT");
					cmdCond = cmdBuffer + pos;					
					cmdCondPointer = cmdCond;
					if (proto_evaluate_cond(&cmdCondPointer)) {
						driver_reset_pos();
						driver_driveStraight(speed, TRIGGER_NONE, 0);
						proto_wait_condition();
						driver_stop();						
					}
					break;
				case TURN_LEFT:
				case TURN_RIGHT:
				{
					lcd_print_string("TURN_LEFT/RIGHT");
					cmdCond = cmdBuffer + (pos + 1);
					driver_reset_pos();
					driver_rotate(speed, ((moveType == TURN_LEFT) ? 1 : -1) * cmdBuffer[pos]);
					cmdCondPointer = cmdCond;
					while(proto_evaluate_cond(&cmdCondPointer)) {					
						drvStatus = driver_status();						
						if(drvStatus->mode == 3) {
							break;
						}												
						cmdCondPointer = cmdCond;
					}
					pos+=1;
					driver_stop();
				}
					break;
			}
			proto_send_done();
			break;
		case READ_VALUE:
			lcd_print_string("READ_VALUE");
			pos++;
			cmdSendBuffer[0] = VALUE;
			cmdSendBuffer[1] = cmdBuffer[pos];
			{
				int val = proto_read_sensor(cmdBuffer[pos]);
				cmdSendBuffer[2] = (byte) (val >> 8);
				cmdSendBuffer[3] = (byte) (val & 0x00FF);
			}
			// proto_dump_message(cmdSendBuffer, 3);
			proto_send(4);
			break;
		case NATIVE_CODE:
			lcd_print_string("NATIVE_CODE");
			asm_call(cmdBuffer + 4);
			proto_send_done();
			break;
		case GET_ENTRY_POINT:
			lcd_print_string("GET_ENTRY_POINT");
			cmdSendBuffer[0] = ENTRY_POINT;
			cmdSendBuffer[1] = ((unsigned int) (cmdBuffer + 1)) >> 8;
			cmdSendBuffer[2] = ((unsigned int) (cmdBuffer + 1)) & 0x00FF;
			proto_send(3);
			break;
		case GET_FUNC_TABLE:
			lcd_print_string("GET_FUNC_TABLE");
			cmdSendBuffer[0] = FUNC_TABLE;
			for (func = 0; func < FUNCTION_LAST; func++) {
				cmdSendBuffer[func * 2 + 1] = ((unsigned int) (asm_getFuncAddress(func))) >> 8;
				cmdSendBuffer[func * 2 + 2] = ((unsigned int) (asm_getFuncAddress(func))) & 0x00FF;
			}
			proto_send(FUNCTION_LAST * 2 + 1);
			break;
	}
	cmdBufferPos = 0;
	cmdBuffer[cmdBufferPos] = '\0';
}
