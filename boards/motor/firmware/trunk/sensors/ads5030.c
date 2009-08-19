#include "ads5030.h"
#include <io.h>
#include <signal.h>

/* CLK is inverted on the ADS5030 board */
#define set_clk(s, x) do { if(!x) P2OUT |= s->clk;	\
		else P2OUT &= ~(s->clk); } while (0)

#define get_bit(s) ((P2IN & (s->dio))?1:0)

void ads_5030_init( sensor_t* sensor,
		    uint8_t clk_n, 
		    uint8_t dio_n )
{
	ads_5030_state_t *s = &(sensor->state.ads5030);

	sensor->read = ads_5030_read;

	s->clk = 1 << clk_n;
	s->dio = 1 << dio_n;

	/* CLK is an output */
	P2DIR |= s->clk;

	/* DIO is an input */
	P2DIR &= ~(s->dio);
}

int32_t ads_5030_read( sensor_t* sensor )
{
	ads_5030_state_t *s = &(sensor->state.ads5030);
	uint8_t i;
	uint16_t d = 0;

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
	s->last_read = d;

	/* TODO: For the moment, just return 0 if the reading is invalid */
	if ( d & 0x4000 )
		return d & 0xff;
	return 0;
}
