#ifndef __COMP_TYPES_H
#define __COMP_TYPES_H

enum { RED, GREEN, BLUE, YELLOW };

typedef enum { GOLF, SQUIRREL } match_type_t;

typedef struct {
	/* Arranged using RED, GREEN, BLUE and YELLOW */
	uint16_t teams[4];
	uint32_t time;
	match_type_t type;
} match_t;

#endif	/* __COMP_TYPES_H */
