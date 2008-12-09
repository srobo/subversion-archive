#include <stdio.h>

/*
This function will return the SMBUS crc8 checksum for the parameter given
From:linux-2.6.1/drivers/i2c/i2c-core.c
*/

#define u8 unsigned char
#define u16 unsigned int

#define POLY    (0x1070U << 3)
static u8 crc8(u16 data){
	int i;
	for(i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ POLY;
		data = data << 1;
	}
	return (u8)(data >> 8);
}

static u8 i2c_smbus_pec(u8 crc, u8 *p, size_t count)
{
        int i;

        for(i = 0; i < count; i++)
                crc = crc8((crc ^ p[i]) << 8);
        return crc;
}


int main(int argc, char **argv){
    u8 tmp[] = {0xAA, 0x0b, 0x06};
    u8 moo;
    moo = i2c_smbus_pec(0, tmp, 3);
    printf("%x\n", moo);
    return;
}
