#ifndef __MOTOR_PLATINE_H__
#define __MOTOR_PLATINE_H__

/**
 * Number of bytes for order-data.
 *
 * The real byte count for the order type is this constant + 1
 * as there is one additional byte for status keeping.
 * This number solely specifies how many bytes a order may have
 * at most.
 * Beware that this should _never_ be set to 0 or less.
 * Best practice is to set it to the max value bytes_needed() in parser.c
 * will return.
 */
#define ORDER_TYPE_MAX_LENGTH 15

/**
 * The I²C Bus Slave Address.
 */
#define TWI_ADDRESS 42

/**
 * The Baud rate for the UART Interface.
 */
#define UART_BAUD_RATE 57600L

#define ORDER_CONTROL 0x01
#define ORDER_CONTROL_RESET 0x11
#define ORDER_CONTROL_STOP_QUEUE 0x21
#define ORDER_CONTROL_CONTINUE_QUEUE 0x31
#define ORDER_CONTROL_CLEAR_QUEUE 0x41
#define ORDER_CONTROL_STOP_DRIVE 0x51
#define ORDER_CONTROL_RESET_TIMER 0x61

#define ORDER_QUERY 0x02
#define ORDER_QUERY_LEFT_SPEED 0x12
#define ORDER_QUERY_RIGHT_SPEED 0x22
#define ORDER_QUERY_QUEUE_SIZE 0x32
#define ORDER_QUERY_CURRENT_ORDER 0x42
#define ORDER_QUERY_TIME_TRIGGER_LEFT 0x52
#define ORDER_QUERY_POS_TRIGGER_LEFT 0x62
#define ORDER_QUERY_TIME_TRIGGER_RIGHT 0x72
#define ORDER_QUERY_POS_TRIGGER_RIGHT 0x82
#define ORDER_QUERY_TIMER 0x92

#define ORDER_DRIVE 0x03
#define ORDER_DRIVE_TRIGGER_LEFT_POSITION 0x20
#define ORDER_DRIVE_TRIGGER_RIGHT_POSITION 0x80
#define ORDER_DRIVE_TRIGGER_LEFT_TIME 0x10
#define ORDER_DRIVE_TRIGGER_RIGHT_TIME 0x40
#define ORDER_DRIVE_N_N 0x03
#define ORDER_DRIVE_T_N 0x13
#define ORDER_DRIVE_P_N 0x23
#define ORDER_DRIVE_N_T 0x43
#define ORDER_DRIVE_T_T 0x53
#define ORDER_DRIVE_P_T 0x63
#define ORDER_DRIVE_N_P 0x83
#define ORDER_DRIVE_T_P 0x93
#define ORDER_DRIVE_P_P 0xa3
#define ORDER_DRIVE_PID 0x33
#define ORDER_DRIVE_SET_DIFF 0xc3

#define ORDER_ADVDRIVE 0x04
#define ORDER_ADVDRIVE_TRIGGER_LEFT_AND 0x20
#define ORDER_ADVDRIVE_TRIGGER_LEFT_OR 0x10
#define ORDER_ADVDRIVE_TRIGGER_RIGHT_AND 0x80
#define ORDER_ADVDRIVE_TRIGGER_RIGHT_OR 0x40
#define ORDER_ADVDRIVE_N_N 0x04
#define ORDER_ADVDRIVE_O_N 0x14
#define ORDER_ADVDRIVE_A_N 0x24
#define ORDER_ADVDRIVE_N_O 0x44
#define ORDER_ADVDRIVE_O_O 0x54
#define ORDER_ADVDRIVE_A_O 0x64
#define ORDER_ADVDRIVE_N_A 0x84
#define ORDER_ADVDRIVE_O_A 0x94
#define ORDER_ADVDRIVE_A_A 0xa4

#define ORDER_SETPID 0x05
#define ORDER_SETPID_BOTH_WHEELS 0x25
#define ORDER_SETPID_LEFT_WHEEL 0x05
#define ORDER_SETPID_RIGHT_WHEEL 0x15

typedef struct ORDER {
	 unsigned char dat[ORDER_TYPE_MAX_LENGTH];
	 unsigned char pos;
} order_t;

void order_init(order_t *order);
unsigned char order_send(order_t *order);
void order_set_type(order_t *order, unsigned char type);
void order_add_params(order_t *order, char *format, ...);
unsigned char order_bytes_to_recv(order_t *order);

#endif
