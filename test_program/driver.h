#ifndef __driver_h__
#define __driver_h__

#ifndef __byte__
#define __byte__
typedef unsigned char byte;
#endif //__byte__

typedef enum {
	TRIGGER_NONE,
	TRIGGER_TIME,
	TRIGGER_POSITION
} driver_trigger;

typedef struct {
	char speedLeft, speedRight;
	char mode; // 3 = stopped, 4 = driving ?
//	int posLeft, posRight;
	int tachoLeft, tachoRight;
} DriverStatus;

void driver_drive(signed char speedLeft, signed char speedRight, driver_trigger trigger,
	int triggerValLeft, int triggerValRight);
void driver_driveStraight(int speed, driver_trigger trigger, int triggerVal);
void driver_rotate(signed char speed, int degree);

DriverStatus *driver_status();

void driver_stop();
void driver_pause();
void driver_unpause();
void driver_moveForward();
void driver_moveBackward();
void driver_rotateLeft();
void driver_rotateRight();

int driver_get_pos();
void driver_save_pos();
void driver_reset_pos();
int driver_pos_to_cm(int pos);

#endif// _driver_h_
