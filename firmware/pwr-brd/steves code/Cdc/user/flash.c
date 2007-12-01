#include <p18cxxx.h>
#include "user/flash.h"

void SetAddress(char high, char low)
{

	//ADD0-5 = RA0-5
	LATA = (LATA & 0xC0) | (low & 0x3F);

	//ADD6-8 = RE0-2)
	//0x78 to make sure bit 7 stays 0 (PORT D Internal pullups!)
	LATE = (LATE & 0xF8) | (((low & 0xC0) >> 6) | ((high & 0x01) << 2));

	//ADD9-11 = RC0-2
	LATC = (LATC & 0xF8) | ((high & 0x0E) >> 1);

	//ADD12-15 = RB0-3
	LATB = (LATB & 0xF0) | ((high & 0xF0) >> 4);
}

void PutData(char d){
	LATCbits.LATC6 = 0;
	LATCbits.LATC6 = 1;
	//This function assumes CE is LOW!!!
    OE = 1;
    WE = 0; //Toggling down latches address
    TRISD = 0;
    LATD = d;
    WE = 1; //Toggling up latches Data
    TRISD = 0xFF;
    OE = 0;
}

unsigned char GetData(){             
	//This function assumes CE and OE are LOW!!!
	//This function assumes WE is HIGH!!!
	TRISD = 0xFF;
    return PORTD;
}

void ResetCmd(){
    //Just have to write 0xF0 to the data register
	PutData(0xF0);
}

void EnterCommandMode(){
    //Step 1 - Set address to 0x555 and write data AA
    SetAddress(0x05, 0x55); //Leaves in reading mode
    PutData(0xAA);
    
    //Step 2 -Set address to 0x2AA and data to 0x55
    SetAddress(0x02, 0xAA);
    PutData(0x55);
}

void EraseSector(char sector){
	//Enter Command mode - 2 steps
	EnterCommandMode();
	
	//Step 3 - Send erase command
	//Address to 0x555 and data ot 0x80
	SetAddress(0x05, 0x55);
	PutData(0x80);
	
	//Steps 4 & 5 - Enter command mode again!
	EnterCommandMode();
	
	//Step 6 - Send Sector Address
	//As ADD16 is tied to 0, ADD15 and ADD14 identify the sector
	//Also send data = 0x30
	SetAddress(sector, 0x00);
	PutData(0x30);
	
	//Now erasing!
	//Wait until Data returns 0xFF, then 100microseconds before
	//The device goes into read mode
	while(GetData() != 0xFF);
	
   //Finished
}

unsigned char GetManufacturerData(){
    char manid, devid;
    //Enter command mode - 2 steps
    EnterCommandMode();
    //Step 3 - Set address to 0x555 and data to 0x90
    SetAddress(0x05, 0x55);
    PutData(0x90);

    //Step 4 - Set address to 00 for manid, 01 for devid and read data!
    SetAddress(0x00, 0x00);
    manid = GetData();
    SetAddress(0x00, 0x01);
    devid = GetData();
    ResetCmd();
   	return devid;
}

unsigned char ProgramData(char high, char low, char data){
    char status;
    
    //Will not check that the sector needs erasing
    //Steps 1 + 2 - Enter command mode
    EnterCommandMode();
    
    //Step 3 - Set address = 0x555 and Data = 0xA0
    SetAddress(0x05, 0x55);
    PutData(0xA0);

    //Step 4 - Write address & data
    SetAddress(high, low);
    PutData(data);

    //Check for complement of D7
    //Following Figure 3 of Flash Data Sheet
    //Initially grab data
    status = GetData();
    //Loop checking bit 7 whilst bit5 is low
    while((status & 0x20) != 0x20){
        //If bit 7 is correct then it's done
        if ((status & 0x80) == (data & 0x80))
            return 0;
        status = GetData();
    }

    //If here then bit 5 has gone high without bit 7 being right.
    //Check again in case they changed simultaneously
    if ((status & 0x80) == (data & 0x80))
            return 0;

    //Definately a failure!
    return 1;
}
    
    
unsigned char GetDataAtAddress(char high, char low){
    SetAddress(high, low);
    return GetData();
}
