#ifndef __COMP_TYPES_H
#define __COMP_TYPES_H

enum { RED, GREEN, BLUE, YELLOW };

typedef struct {
	/* Arranged using RED, GREEN, BLUE and YELLOW */
	uint16_t teams[4];
	uint32_t time;
} match_t;

#endif	/* __COMP_TYPES_H */
