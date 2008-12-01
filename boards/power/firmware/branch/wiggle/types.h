#ifndef __TYPES_H
#define __TYPES_H
#include <sys/types.h>

typedef enum
  {
    FALSE = 0,
    TRUE
  } bool;

#define BIT(n) ( 1 << (n) )


#endif	/* __TYPES_H */
