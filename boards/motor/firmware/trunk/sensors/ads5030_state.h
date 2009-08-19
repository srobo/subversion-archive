#ifndef __ADS5030_STATE_H
#define __ADS5030_STATE_H

typedef struct {
	/* Bit masks for CLK and DIO (Against P2{OUT,DIR,IN}) */
	uint8_t clk, dio;

	/* The last reading (debug) */
	uint16_t last_read;
} ads_5030_state_t;

#endif	/* __ADS5030_STATE_H */
