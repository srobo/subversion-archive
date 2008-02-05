#define ADD0 LATAbits.LATA0
#define ADD1 LATAbits.LATA1
#define ADD2 LATAbits.LATA2
#define ADD3 LATAbits.LATA3
#define ADD4 LATAbits.LATA4
#define ADD5 LATAbits.LATA5

#define ADD12 LATBbits.LATB0
#define ADD13 LATBbits.LATB1
#define ADD14 LATBbits.LATB2
#define ADD15 LATBbits.LATB3
#define OE LATBbits.LATB4
#define WE LATBbits.LATB5

#define ADD9 LATCbits.LATC0
#define ADD10 LATCbits.LATC1
#define ADD11 LATCbits.LATC2
#define USB_CONNECTED LATCbits.LATB6
#define PC_CONNECTED LATCbits.LATB7

#define DATA0 LATDbits.LATD0
#define DATA1 LATDbits.LATD1
#define DATA2 LATDbits.LATD2
#define DATA3 LATDbits.LATD3
#define DATA4 LATDbits.LATD4
#define DATA5 LATDbits.LATD5
#define DATA6 LATDbits.LATD6
#define DATA7 LATDbits.LATD7

#define ADD6 LATEbits.LATE0
#define ADD7 LATEbits.LATE1
#define ADD8 LATEbits.LATE2

void SetAddress(char high, char low);
void PutData(char d);
unsigned char GetData( void );
void ResetCmd( void );
void EnterCommandMode( void );
void EraseSector(char sector);
unsigned char GetManufacturerData( void );
unsigned char ProgramData(char high, char low, char data);
unsigned char GetDataAtAddress(char high, char low);
