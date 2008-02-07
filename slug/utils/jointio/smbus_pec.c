#include "smbus_pec.h"

uint8_t crc8( uint8_t tempdata )
{
        uint8_t i;
        uint16_t data;

        data = (uint16_t)tempdata<<8;
        for(i = 0; i < 8; i++)
        {
                if (data & 0x8000)
                        data = data ^ SMBUS_POLY;
                data <<= 1;
        }

        return (data >> 8) & 0xFF;
}
