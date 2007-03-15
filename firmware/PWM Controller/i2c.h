/**i2c for msp430
header file
**/

#ifndef I2C_HEADER
#define I2C_HEADER

#define ADDRESS 0x1E

char available_i2c_data_number(void);
char available_i2c_data(void);
char * get_i2cData(void);
void initialise_i2c(void);
void enable_i2c(void);
void isr_usi (void);
char smbus_parse(char command);



#endif
