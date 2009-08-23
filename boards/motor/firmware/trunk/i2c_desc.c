#include "i2c_desc.h"
#include "common.h"

void i2c_desc_write( uint8_t* data, 
		     const i2c_setting_t *setting,
		     void *obj )
{
	void *s = obj + setting->def.s;

	switch( setting->type ) {
	case ST_U8:
	case ST_I8:
		*(uint8_t*)s = *data;
		break;

	case ST_U16:
	case ST_I16:
		*(uint16_t*)s = (uint16_t)data[0]
			| (uint16_t)data[1] << 8;
		break;

	case ST_U32:
	case ST_I32:
		*(uint32_t*)s = (uint32_t)data[0]
			| (uint32_t)data[1] << 8
			| (uint32_t)data[2] << 16
			| (uint32_t)data[3] << 24;
		break;

	case ST_BOOL:
		if( data[0] )
			*(uint8_t*)s = 1;
		else
			*(uint8_t*)s = 0;
		break;

	case ST_FUNC:
		setting->def.funcs.rx( data );
		break;
	}
}

uint8_t i2c_desc_read( uint8_t* data, const i2c_setting_t *setting, void *obj )
{
	void *s = obj + setting->def.s;

	switch( setting->type ) {
	case ST_U8:
	case ST_I8:
		data[0] = *(uint8_t*)s;
		return 1;

	case ST_U16:
	case ST_I16:
		data[0] = *(uint16_t*)s & 0xff;
		data[1] = (*(uint16_t*)s >> 8) & 0xff;
		return 2;

	case ST_U32:
	case ST_I32:
		data[0] = *(uint32_t*)s & 0xff;
		data[1] = (*(uint32_t*)s >> 8) & 0xff;
		data[2] = (*(uint32_t*)s >> 16) & 0xff;
		data[3] = (*(uint32_t*)s >> 24) & 0xff;
		return 4;

	case ST_BOOL:
		if( *(uint8_t*)s )
			data[0] = 1;
		else
			data[0] = 0;
		return 1;

	case ST_FUNC:
		if( setting->def.funcs.tx != NULL )
			return setting->def.funcs.tx( data );
		return 0;
	}

	return 0;
}

uint8_t i2c_desc_get_rx_size( const i2c_setting_t* s )
{
	switch( s->type ) {
	case ST_U8:
	case ST_I8:
		return 1;

	case ST_U16:
	case ST_I16:
		return 2;

	case ST_U32:
	case ST_I32:
		return 4;

	case ST_BOOL:
		return 1;

	case ST_FUNC:
		return s->def.funcs.rx_size;

	default:
		return 0;
	}
}

