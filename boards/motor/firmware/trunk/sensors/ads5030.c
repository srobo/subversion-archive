#include "ads5030.h"
#include "../i2c_desc.h"
#include <io.h>
#include <signal.h>

/* CLK is inverted on the ADS5030 board */
#define set_clk(s, x) do { if(!x) P2OUT |= s->clk;	\
		else P2OUT &= ~(s->clk); } while (0)

#define get_bit(s) ((P2IN & (s->dio))?1:0)

static const i2c_setting_t ads5030_settings[] = {
	/* clk and dio are bit *masks* */
	I2C_DESC_SETTING( ST_U8, ads_5030_state_t, clk ),
	I2C_DESC_SETTING( ST_U8, ads_5030_state_t, dio ),
	I2C_DESC_SETTING( ST_U8, ads_5030_state_t, shr )
};

/* Returns the angular position */
/* Returns 0xffff on fail */
static uint16_t ads5030_read_angle( ads_5030_state_t* s );

static int32_t ads_5030_read( sensor_t* sensor );

void ads_5030_init( sensor_t* sensor )
{
	ads_5030_state_t *s = &(sensor->state.ads5030);

	sensor->read = ads_5030_read;

	s->clk = 0;
	s->dio = 0;
	s->shr = 0;

	s->last_read = 0;
	s->pos = 0;

	/* CLK is an output */
	P2DIR |= s->clk;

	/* DIO is an input */
	P2DIR &= ~(s->dio);

	sensor->i2c_tbl = ads5030_settings;
	sensor->i2c_tblen = sizeof( ads5030_settings ) / sizeof( i2c_setting_t );
}

static int32_t ads_5030_read( sensor_t* sensor )
{
	ads_5030_state_t *s = &(sensor->state.ads5030);
	uint16_t r = ads5030_read_angle( s );
	const int32_t max = (255 >> s->shr);
	int32_t x;

	/* Nothing we can do with failed comms with sensor :-( */
	if( r == 0xffff )
		return s->pos;

	x = ((int16_t)r) - ((int16_t)s->last_read);

	/* Compensate for the discontinuity */
	/* We assume that the wheel hasn't turned more than half a turn */
	if( x > (max/2) )
		x -= max;
	else if ( x < (0-(max/2)) )
		x += max;

	/* No discontinuity to deal with */
	s->pos += x;

	s->last_read = r;

	return s->pos;
}

static uint16_t ads5030_read_angle( ads_5030_state_t* s )
{
	uint8_t i;
	uint16_t d = 0;

	/*** Set up the IO ***/
	/* TODO: Do this when the values are configured */
	/* CLK is an output */
	P2DIR |= s->clk;

	/* DIO is an input */
	P2DIR &= ~(s->dio);

	/* In 2-wire mode, the device times out after 20us,
	   so avoid any nasty things happening */
	dint();

	set_clk(s, 1);
	nop();
	for( i=0; i<5; i++ ) {
		set_clk(s, 0);
		nop();
		set_clk(s, 1);
		nop();
	}		
	nop();

	for( i=0; i<16; i++ ) {
		set_clk(s, 0);
		d <<= 1;

		set_clk(s, 1);
		d |= get_bit(s);
	}
	set_clk(s, 0);

	eint();

	if ( d & 0x4000 )
		return (d & 0xff) >> s->shr;

	/* Reading invalid. */
	return 0xffff;
}
