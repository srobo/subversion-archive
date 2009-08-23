#ifndef __I2C_DESC_H
#define __I2C_DESC_H
#include <stdint.h>
#include <stddef.h>

typedef enum {
	ST_U8,
	ST_U16,
	ST_U32,
	ST_I8,
	ST_I16,
	ST_I32,
	ST_BOOL,
	ST_FUNC
} i2c_stype_t;

typedef struct {
	/* The type of setting */
	i2c_stype_t type;

	union {
		/* Offset of the setting */
		uint16_t s;

		struct {
			/* The receive size - 0 if receive not supported*/
			uint8_t rx_size;

			/* Receive function - processes buf data */
			void (*rx) ( uint8_t* buf );

			/* Transmit function - fills buf with data.
			   Returns length of the data. */
			uint8_t (*tx) ( uint8_t* buf );
		} funcs;
	} def;
} i2c_setting_t;

#define I2C_DESC_FUNC( RX_SIZE, RX, TX ) { ST_FUNC, \
			.def = { .funcs = { RX_SIZE, RX, TX } } }

#define I2C_DESC_SETTING( TYPE, STRUCTURE, MEMBER ) { TYPE, \
			.def = { .s = offsetof( STRUCTURE, MEMBER ) } }

uint8_t i2c_desc_get_rx_size( const i2c_setting_t* s );

/* Writes the setting */
void i2c_desc_write( uint8_t* data, const i2c_setting_t *setting, void *obj );
/* Reads the setting into the buffer pointer to by data */
uint8_t i2c_desc_read( uint8_t* data, const i2c_setting_t *setting, void *obj );

#endif	/* __I2C_DESC_H */
