#ifndef __ADS5030_STATE_H
#define __ADS5030_STATE_H

typedef struct {
	/* Bit masks for CLK and DIO (Against P2{OUT,DIR,IN}) */
	uint8_t clk, dio;

	/* How many LSbits to shift out */
	uint8_t shr;

	/* The last angle -- for tracking more than one rotation */
	uint8_t last_read;

	/* The tracked position */
	int32_t pos;
} ads_5030_state_t;

#endif	/* __ADS5030_STATE_H */
