#ifndef __PWR_QUAL_H
#define __PWR_QUAL_H


#define ADDRESS 0x4f

typedef enum
{
	FALSE = 0, TRUE
} bool;

typedef enum
{
	IDENTIFY,
	LED,
	DIPSWITCH,
	BUTTON,
	SLUG_POWER,
	SERVO_POWER,
	MOTOR_POWER,
	BATTERY,
	VOLT,
	AMP,
	BEEGEES,
	TEST,
	BUTTON_FAKE
} com;
bool err_enable = TRUE;

#endif
