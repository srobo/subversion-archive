#ifndef __PWR-QUAL_H
#define __PWR-QUAL_H




/* 	I2C_REG_ENTRY_RO( identify ),  */
/* 	I2C_REG_ENTRY( led ),	       */
/* 	I2C_REG_ENTRY_RO( dipswitch ),  */
/* 	I2C_REG_ENTRY_RO( button ), */
/* 	I2C_REG_ENTRY( slug_power ),   */
/* 	I2C_REG_ENTRY_WO( servo_power ),	 */
/* 	I2C_REG_ENTRY( motor_power ),	    */
/* 	I2C_REG_ENTRY_RO( battery ),    */
/* 	I2C_REG_ENTRY_WO( beegees ),   */


//#define ADDRESS 0x55
#define ADDRESS 0x3f

typedef enum
{
	FALSE = 0, TRUE
} bool;

typedef enum
{
	IDENTIFY,
	SETLED,
	GETV,
	GETI,
	GETDIPS,
	SETRAILS,
	GETRAILS,
	SENDSER,
	ISUSB,
	BEEGEES,
} com;
bool err_enable = TRUE;

#endif
