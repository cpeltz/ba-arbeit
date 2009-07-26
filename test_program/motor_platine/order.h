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

#define ORDER_REGISTER 0x02
#define ORDER_REGISTER_LEFT_SPEED 0x12
#define ORDER_REGISTER_RIGHT_SPEED 0x22
#define ORDER_REGISTER_QUEUE_SIZE 0x32
#define ORDER_REGISTER_CURRENT_ORDER 0x42

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
unsigned char order_send_and_recv(order_t *order); 
unsigned char order_send_and_recv_co(order_t *order);

/*order_t order;
order_init(&order);
order.data[0] = ORDER_CONTROL_RESET;
order_send(&order);

order_t order;
order_init(&order);
order.data[0] = ORDER_DRIVE + ORDER_DRIVE_LEFT_POSITION + ORDER_DRIVE_RIGHT_TIME;
order.data[1] = 127;
order.data[2] = -15;
order.data[3] = 1450 >> 8;
order.data[4] = (1450 << 8) >> 8;
order.data[5] = 30000 >> 8;
order.data[6] = (30000 << 8) >> 8;
order_send(&order);

order_t order;
order_init(&order);
order_set_type(ORDER_DRIVE + ORDER_DRIVE_LEFT_POSITION + ORDER_DRIVE_RIGHT_TIME);
order_add(127);
order_add(-15);
order_add2(1450);
order_add2(30000);
order_send(&order);

order_t order;
order_init(&order);
order_set_type(ORDER_DRIVE + ORDER_DRIVE_LEFT_POSITION + ORDER_DRIVE_RIGHT_TIME);
order_add_params("1122", 127, -15, 1450, 3000);
order_send(&order);*/

#endif
