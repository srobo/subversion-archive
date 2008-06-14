#line 1 "usb9.c"

#line 35 "usb9.c"
 

 
#line 1 "C:/mcc18/h/p18cxxx.h"

#line 3 "C:/mcc18/h/p18cxxx.h"

#line 5 "C:/mcc18/h/p18cxxx.h"
#line 7 "C:/mcc18/h/p18cxxx.h"
#line 9 "C:/mcc18/h/p18cxxx.h"
#line 11 "C:/mcc18/h/p18cxxx.h"
#line 13 "C:/mcc18/h/p18cxxx.h"
#line 15 "C:/mcc18/h/p18cxxx.h"
#line 17 "C:/mcc18/h/p18cxxx.h"
#line 19 "C:/mcc18/h/p18cxxx.h"
#line 21 "C:/mcc18/h/p18cxxx.h"
#line 23 "C:/mcc18/h/p18cxxx.h"
#line 25 "C:/mcc18/h/p18cxxx.h"
#line 27 "C:/mcc18/h/p18cxxx.h"
#line 29 "C:/mcc18/h/p18cxxx.h"
#line 31 "C:/mcc18/h/p18cxxx.h"
#line 33 "C:/mcc18/h/p18cxxx.h"
#line 35 "C:/mcc18/h/p18cxxx.h"
#line 37 "C:/mcc18/h/p18cxxx.h"
#line 39 "C:/mcc18/h/p18cxxx.h"
#line 41 "C:/mcc18/h/p18cxxx.h"
#line 43 "C:/mcc18/h/p18cxxx.h"
#line 45 "C:/mcc18/h/p18cxxx.h"
#line 47 "C:/mcc18/h/p18cxxx.h"
#line 49 "C:/mcc18/h/p18cxxx.h"
#line 51 "C:/mcc18/h/p18cxxx.h"
#line 53 "C:/mcc18/h/p18cxxx.h"
#line 55 "C:/mcc18/h/p18cxxx.h"
#line 57 "C:/mcc18/h/p18cxxx.h"
#line 59 "C:/mcc18/h/p18cxxx.h"
#line 61 "C:/mcc18/h/p18cxxx.h"
#line 63 "C:/mcc18/h/p18cxxx.h"
#line 65 "C:/mcc18/h/p18cxxx.h"
#line 67 "C:/mcc18/h/p18cxxx.h"
#line 69 "C:/mcc18/h/p18cxxx.h"
#line 71 "C:/mcc18/h/p18cxxx.h"
#line 73 "C:/mcc18/h/p18cxxx.h"
#line 75 "C:/mcc18/h/p18cxxx.h"
#line 77 "C:/mcc18/h/p18cxxx.h"
#line 79 "C:/mcc18/h/p18cxxx.h"
#line 81 "C:/mcc18/h/p18cxxx.h"
#line 83 "C:/mcc18/h/p18cxxx.h"
#line 85 "C:/mcc18/h/p18cxxx.h"
#line 87 "C:/mcc18/h/p18cxxx.h"
#line 89 "C:/mcc18/h/p18cxxx.h"
#line 91 "C:/mcc18/h/p18cxxx.h"
#line 93 "C:/mcc18/h/p18cxxx.h"
#line 95 "C:/mcc18/h/p18cxxx.h"
#line 97 "C:/mcc18/h/p18cxxx.h"
#line 99 "C:/mcc18/h/p18cxxx.h"
#line 101 "C:/mcc18/h/p18cxxx.h"
#line 103 "C:/mcc18/h/p18cxxx.h"
#line 105 "C:/mcc18/h/p18cxxx.h"
#line 107 "C:/mcc18/h/p18cxxx.h"
#line 109 "C:/mcc18/h/p18cxxx.h"
#line 111 "C:/mcc18/h/p18cxxx.h"
#line 113 "C:/mcc18/h/p18cxxx.h"
#line 115 "C:/mcc18/h/p18cxxx.h"
#line 117 "C:/mcc18/h/p18cxxx.h"
#line 119 "C:/mcc18/h/p18cxxx.h"
#line 121 "C:/mcc18/h/p18cxxx.h"
#line 123 "C:/mcc18/h/p18cxxx.h"
#line 125 "C:/mcc18/h/p18cxxx.h"
#line 127 "C:/mcc18/h/p18cxxx.h"
#line 129 "C:/mcc18/h/p18cxxx.h"
#line 131 "C:/mcc18/h/p18cxxx.h"
#line 133 "C:/mcc18/h/p18cxxx.h"
#line 135 "C:/mcc18/h/p18cxxx.h"
#line 137 "C:/mcc18/h/p18cxxx.h"
#line 139 "C:/mcc18/h/p18cxxx.h"
#line 1 "C:/mcc18/h/p18f4550.h"

#line 6 "C:/mcc18/h/p18f4550.h"
 


#line 10 "C:/mcc18/h/p18f4550.h"

extern volatile near unsigned char       SPPDATA;
extern          near unsigned char       SPPCFG;
extern          near struct {
  unsigned WS0:1;
  unsigned WS1:1;
  unsigned WS2:1;
  unsigned WS3:1;
  unsigned CLK1EN:1;
  unsigned CSEN:1;
  unsigned CLKCFG0:1;
  unsigned CLKCFG1:1;
} SPPCFGbits;
extern          near unsigned char       SPPEPS;
extern          near struct {
  unsigned ADDR0:1;
  unsigned ADDR1:1;
  unsigned ADDR2:1;
  unsigned ADDR3:1;
  unsigned SPPBUSY:1;
  unsigned :1;
  unsigned WRSPP:1;
  unsigned RDSPP:1;
} SPPEPSbits;
extern          near unsigned char       SPPCON;
extern          near struct {
  unsigned SPPEN:1;
  unsigned SPPOWN:1;
} SPPCONbits;
extern volatile near unsigned            UFRM;
extern volatile near unsigned char       UFRML;
extern volatile near unsigned char       UFRMH;
extern volatile near unsigned char       UIR;
extern volatile near struct {
  unsigned URSTIF:1;
  unsigned UERRIF:1;
  unsigned ACTVIF:1;
  unsigned TRNIF:1;
  unsigned IDLEIF:1;
  unsigned STALLIF:1;
  unsigned SOFIF:1;
} UIRbits;
extern volatile near unsigned char       UIE;
extern volatile near struct {
  unsigned URSTIE:1;
  unsigned UERRIE:1;
  unsigned ACTVIE:1;
  unsigned TRNIE:1;
  unsigned IDLEIE:1;
  unsigned STALLIE:1;
  unsigned SOFIE:1;
} UIEbits;
extern volatile near unsigned char       UEIR;
extern volatile near struct {
  unsigned PIDEF:1;
  unsigned CRC5EF:1;
  unsigned CRC16EF:1;
  unsigned DFN8EF:1;
  unsigned BTOEF:1;
  unsigned :2;
  unsigned BTSEF:1;
} UEIRbits;
extern volatile near unsigned char       UEIE;
extern volatile near struct {
  unsigned PIDEE:1;
  unsigned CRC5EE:1;
  unsigned CRC16EE:1;
  unsigned DFN8EE:1;
  unsigned BTOEE:1;
  unsigned :2;
  unsigned BTSEE:1;
} UEIEbits;
extern volatile near unsigned char       USTAT;
extern volatile near struct {
  unsigned :1;
  unsigned PPBI:1;
  unsigned DIR:1;
  unsigned ENDP0:1;
  unsigned ENDP1:1;
  unsigned ENDP2:1;
  unsigned ENDP3:1;
} USTATbits;
extern          near unsigned char       UCON;
extern          near struct {
  unsigned :1;
  unsigned SUSPND:1;
  unsigned RESUME:1;
  unsigned USBEN:1;
  unsigned PKTDIS:1;
  unsigned SE0:1;
  unsigned PPBRST:1;
} UCONbits;
extern          near unsigned char       UADDR;
extern          near struct {
  unsigned ADDR0:1;
  unsigned ADDR1:1;
  unsigned ADDR2:1;
  unsigned ADDR3:1;
  unsigned ADDR4:1;
  unsigned ADDR5:1;
  unsigned ADDR6:1;
} UADDRbits;
extern          near unsigned char       UCFG;
extern          near struct {
  unsigned PPB0:1;
  unsigned PPB1:1;
  unsigned FSEN:1;
  unsigned UTRDIS:1;
  unsigned UPUEN:1;
  unsigned :1;
  unsigned UOEMON:1;
  unsigned UTEYE:1;
} UCFGbits;
extern          near unsigned char       UEP0;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP0bits;
extern          near unsigned char       UEP1;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP1bits;
extern          near unsigned char       UEP2;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP2bits;
extern          near unsigned char       UEP3;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP3bits;
extern          near unsigned char       UEP4;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP4bits;
extern          near unsigned char       UEP5;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP5bits;
extern          near unsigned char       UEP6;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP6bits;
extern          near unsigned char       UEP7;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP7bits;
extern          near unsigned char       UEP8;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP8bits;
extern          near unsigned char       UEP9;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP9bits;
extern          near unsigned char       UEP10;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP10bits;
extern          near unsigned char       UEP11;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP11bits;
extern          near unsigned char       UEP12;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP12bits;
extern          near unsigned char       UEP13;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP13bits;
extern          near unsigned char       UEP14;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP14bits;
extern          near unsigned char       UEP15;
extern          near struct {
  unsigned EPSTALL:1;
  unsigned EPINEN:1;
  unsigned EPOUTEN:1;
  unsigned EPCONDIS:1;
  unsigned EPHSHK:1;
} UEP15bits;
extern volatile near unsigned char       PORTA;
extern volatile near union {
  struct {
    unsigned RA0:1;
    unsigned RA1:1;
    unsigned RA2:1;
    unsigned RA3:1;
    unsigned RA4:1;
    unsigned RA5:1;
    unsigned RA6:1;
  };
  struct {
    unsigned AN0:1;
    unsigned AN1:1;
    unsigned AN2:1;
    unsigned AN3:1;
    unsigned T0CKI:1;
    unsigned AN4:1;
    unsigned OSC2:1;
  };
  struct {
    unsigned :2;
    unsigned VREFM:1;
    unsigned VREFP:1;
    unsigned :1;
    unsigned LVDIN:1;
  };
  struct {
    unsigned :5;
    unsigned HLVDIN:1;
  };
} PORTAbits;
extern volatile near unsigned char       PORTB;
extern volatile near union {
  struct {
    unsigned RB0:1;
    unsigned RB1:1;
    unsigned RB2:1;
    unsigned RB3:1;
    unsigned RB4:1;
    unsigned RB5:1;
    unsigned RB6:1;
    unsigned RB7:1;
  };
  struct {
    unsigned INT0:1;
    unsigned INT1:1;
    unsigned INT2:1;
  };
  struct {
    unsigned :5;
    unsigned PGM:1;
    unsigned PGC:1;
    unsigned PGD:1;
  };
} PORTBbits;
extern volatile near unsigned char       PORTC;
extern volatile near union {
  struct {
    unsigned RC0:1;
    unsigned RC1:1;
    unsigned RC2:1;
    unsigned :1;
    unsigned RC4:1;
    unsigned RC5:1;
    unsigned RC6:1;
    unsigned RC7:1;
  };
  struct {
    unsigned T1OSO:1;
    unsigned T1OSI:1;
    unsigned CCP1:1;
    unsigned :3;
    unsigned TX:1;
    unsigned RX:1;
  };
  struct {
    unsigned T13CKI:1;
    unsigned :1;
    unsigned P1A:1;
    unsigned :3;
    unsigned CK:1;
    unsigned DT:1;
  };
} PORTCbits;
extern volatile near unsigned char       PORTD;
extern volatile near union {
  struct {
    unsigned RD0:1;
    unsigned RD1:1;
    unsigned RD2:1;
    unsigned RD3:1;
    unsigned RD4:1;
    unsigned RD5:1;
    unsigned RD6:1;
    unsigned RD7:1;
  };
  struct {
    unsigned SPP0:1;
    unsigned SPP1:1;
    unsigned SPP2:1;
    unsigned SPP3:1;
    unsigned SPP4:1;
    unsigned SPP5:1;
    unsigned SPP6:1;
    unsigned SPP7:1;
  };
} PORTDbits;
extern volatile near unsigned char       PORTE;
extern volatile near union {
  struct {
    unsigned RE0:1;
    unsigned RE1:1;
    unsigned RE2:1;
    unsigned RE3:1;
    unsigned :3;
    unsigned RDPU:1;
  };
  struct {
    unsigned CK1SPP:1;
    unsigned CK2SPP:1;
    unsigned OESPP:1;
  };
} PORTEbits;
extern volatile near unsigned char       LATA;
extern volatile near struct {
  unsigned LATA0:1;
  unsigned LATA1:1;
  unsigned LATA2:1;
  unsigned LATA3:1;
  unsigned LATA4:1;
  unsigned LATA5:1;
  unsigned LATA6:1;
} LATAbits;
extern volatile near unsigned char       LATB;
extern volatile near struct {
  unsigned LATB0:1;
  unsigned LATB1:1;
  unsigned LATB2:1;
  unsigned LATB3:1;
  unsigned LATB4:1;
  unsigned LATB5:1;
  unsigned LATB6:1;
  unsigned LATB7:1;
} LATBbits;
extern volatile near unsigned char       LATC;
extern volatile near struct {
  unsigned LATC0:1;
  unsigned LATC1:1;
  unsigned LATC2:1;
  unsigned :3;
  unsigned LATC6:1;
  unsigned LATC7:1;
} LATCbits;
extern volatile near unsigned char       LATD;
extern volatile near struct {
  unsigned LATD0:1;
  unsigned LATD1:1;
  unsigned LATD2:1;
  unsigned LATD3:1;
  unsigned LATD4:1;
  unsigned LATD5:1;
  unsigned LATD6:1;
  unsigned LATD7:1;
} LATDbits;
extern volatile near unsigned char       LATE;
extern volatile near struct {
  unsigned LATE0:1;
  unsigned LATE1:1;
  unsigned LATE2:1;
} LATEbits;
extern volatile near unsigned char       DDRA;
extern volatile near struct {
  unsigned RA0:1;
  unsigned RA1:1;
  unsigned RA2:1;
  unsigned RA3:1;
  unsigned RA4:1;
  unsigned RA5:1;
  unsigned RA6:1;
} DDRAbits;
extern volatile near unsigned char       TRISA;
extern volatile near struct {
  unsigned TRISA0:1;
  unsigned TRISA1:1;
  unsigned TRISA2:1;
  unsigned TRISA3:1;
  unsigned TRISA4:1;
  unsigned TRISA5:1;
  unsigned TRISA6:1;
} TRISAbits;
extern volatile near unsigned char       DDRB;
extern volatile near struct {
  unsigned RB0:1;
  unsigned RB1:1;
  unsigned RB2:1;
  unsigned RB3:1;
  unsigned RB4:1;
  unsigned RB5:1;
  unsigned RB6:1;
  unsigned RB7:1;
} DDRBbits;
extern volatile near unsigned char       TRISB;
extern volatile near struct {
  unsigned TRISB0:1;
  unsigned TRISB1:1;
  unsigned TRISB2:1;
  unsigned TRISB3:1;
  unsigned TRISB4:1;
  unsigned TRISB5:1;
  unsigned TRISB6:1;
  unsigned TRISB7:1;
} TRISBbits;
extern volatile near unsigned char       DDRC;
extern volatile near struct {
  unsigned RC0:1;
  unsigned RC1:1;
  unsigned RC2:1;
  unsigned :3;
  unsigned RC6:1;
  unsigned RC7:1;
} DDRCbits;
extern volatile near unsigned char       TRISC;
extern volatile near struct {
  unsigned TRISC0:1;
  unsigned TRISC1:1;
  unsigned TRISC2:1;
  unsigned :3;
  unsigned TRISC6:1;
  unsigned TRISC7:1;
} TRISCbits;
extern volatile near unsigned char       DDRD;
extern volatile near struct {
  unsigned RD0:1;
  unsigned RD1:1;
  unsigned RD2:1;
  unsigned RD3:1;
  unsigned RD4:1;
  unsigned RD5:1;
  unsigned RD6:1;
  unsigned RD7:1;
} DDRDbits;
extern volatile near unsigned char       TRISD;
extern volatile near struct {
  unsigned TRISD0:1;
  unsigned TRISD1:1;
  unsigned TRISD2:1;
  unsigned TRISD3:1;
  unsigned TRISD4:1;
  unsigned TRISD5:1;
  unsigned TRISD6:1;
  unsigned TRISD7:1;
} TRISDbits;
extern volatile near unsigned char       DDRE;
extern volatile near struct {
  unsigned RE0:1;
  unsigned RE1:1;
  unsigned RE2:1;
} DDREbits;
extern volatile near unsigned char       TRISE;
extern volatile near struct {
  unsigned TRISE0:1;
  unsigned TRISE1:1;
  unsigned TRISE2:1;
} TRISEbits;
extern volatile near unsigned char       OSCTUNE;
extern volatile near struct {
  unsigned TUN0:1;
  unsigned TUN1:1;
  unsigned TUN2:2;
  unsigned TUN3:1;
  unsigned TUN4:1;
  unsigned :2;
  unsigned INTSRC:1;
} OSCTUNEbits;
extern volatile near unsigned char       PIE1;
extern volatile near struct {
  unsigned TMR1IE:1;
  unsigned TMR2IE:1;
  unsigned CCP1IE:1;
  unsigned SSPIE:1;
  unsigned TXIE:1;
  unsigned RCIE:1;
  unsigned ADIE:1;
  unsigned SPPIE:1;
} PIE1bits;
extern volatile near unsigned char       PIR1;
extern volatile near struct {
  unsigned TMR1IF:1;
  unsigned TMR2IF:1;
  unsigned CCP1IF:1;
  unsigned SSPIF:1;
  unsigned TXIF:1;
  unsigned RCIF:1;
  unsigned ADIF:1;
  unsigned SPPIF:1;
} PIR1bits;
extern volatile near unsigned char       IPR1;
extern volatile near struct {
  unsigned TMR1IP:1;
  unsigned TMR2IP:1;
  unsigned CCP1IP:1;
  unsigned SSPIP:1;
  unsigned TXIP:1;
  unsigned RCIP:1;
  unsigned ADIP:1;
  unsigned SPPIP:1;
} IPR1bits;
extern volatile near unsigned char       PIE2;
extern volatile near union {
  struct {
    unsigned CCP2IE:1;
    unsigned TMR3IE:1;
    unsigned LVDIE:1;
    unsigned BCLIE:1;
    unsigned EEIE:1;
    unsigned USBIE:1;
    unsigned CMIE:1;
    unsigned OSCFIE:1;
  };
  struct {
    unsigned :2;
    unsigned HLVDIE:1;
  };
} PIE2bits;
extern volatile near unsigned char       PIR2;
extern volatile near union {
  struct {
    unsigned CCP2IF:1;
    unsigned TMR3IF:1;
    unsigned LVDIF:1;
    unsigned BCLIF:1;
    unsigned EEIF:1;
    unsigned USBIF:1;
    unsigned CMIF:1;
    unsigned OSCFIF:1;
  };
  struct {
    unsigned :2;
    unsigned HLVDIF:1;
  };
} PIR2bits;
extern volatile near unsigned char       IPR2;
extern volatile near union {
  struct {
    unsigned CCP2IP:1;
    unsigned TMR3IP:1;
    unsigned LVDIP:1;
    unsigned BCLIP:1;
    unsigned EEIP:1;
    unsigned USBIP:1;
    unsigned CMIP:1;
    unsigned OSCFIP:1;
  };
  struct {
    unsigned :2;
    unsigned HLVDIP:1;
  };
} IPR2bits;
extern volatile near unsigned char       EECON1;
extern volatile near struct {
  unsigned RD:1;
  unsigned WR:1;
  unsigned WREN:1;
  unsigned WRERR:1;
  unsigned FREE:1;
  unsigned :1;
  unsigned CFGS:1;
  unsigned EEPGD:1;
} EECON1bits;
extern volatile near unsigned char       EECON2;
extern volatile near unsigned char       EEDATA;
extern volatile near unsigned char       EEADR;
extern volatile near unsigned char       RCSTA;
extern volatile near union {
  struct {
    unsigned RX9D:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned ADDEN:1;
    unsigned CREN:1;
    unsigned SREN:1;
    unsigned RX9:1;
    unsigned SPEN:1;
  };
  struct {
    unsigned :3;
    unsigned ADEN:1;
  };
} RCSTAbits;
extern volatile near unsigned char       TXSTA;
extern volatile near struct {
  unsigned TX9D:1;
  unsigned TRMT:1;
  unsigned BRGH:1;
  unsigned SENDB:1;
  unsigned SYNC:1;
  unsigned TXEN:1;
  unsigned TX9:1;
  unsigned CSRC:1;
} TXSTAbits;
extern volatile near unsigned char       TXREG;
extern volatile near unsigned char       RCREG;
extern volatile near unsigned char       SPBRG;
extern volatile near unsigned char       SPBRGH;
extern volatile near unsigned char       T3CON;
extern volatile near union {
  struct {
    unsigned TMR3ON:1;
    unsigned TMR3CS:1;
    unsigned T3SYNC:1;
    unsigned T3CCP1:1;
    unsigned T3CKPS0:1;
    unsigned T3CKPS1:1;
    unsigned T3CCP2:1;
    unsigned RD16:1;
  };
  struct {
    unsigned :2;
    unsigned T3NSYNC:1;
  };
  struct {
    unsigned :2;
    unsigned NOT_T3SYNC:1;
  };
} T3CONbits;
extern volatile near unsigned char       TMR3L;
extern volatile near unsigned char       TMR3H;
extern volatile near unsigned char       CMCON;
extern volatile near struct {
  unsigned CM0:1;
  unsigned CM1:1;
  unsigned CM2:1;
  unsigned CIS:1;
  unsigned C1INV:1;
  unsigned C2INV:1;
  unsigned C1OUT:1;
  unsigned C2OUT:1;
} CMCONbits;
extern volatile near unsigned char       CVRCON;
extern volatile near union {
  struct {
    unsigned CVR0:1;
    unsigned CVR1:1;
    unsigned CVR2:1;
    unsigned CVR3:1;
    unsigned CVREF:1;
    unsigned CVRR:1;
    unsigned CVROE:1;
    unsigned CVREN:1;
  };
  struct {
    unsigned :4;
    unsigned CVRSS:1;
  };
} CVRCONbits;
extern volatile near unsigned char       CCP1AS;
extern volatile near struct {
  unsigned :2;
  unsigned PSSAC0:1;
  unsigned PSSAC1:1;
  unsigned ECCPAS0:1;
  unsigned ECCPAS1:1;
  unsigned ECCPAS2:1;
  unsigned ECCPASE:1;
} CCP1ASbits;
extern volatile near unsigned char       ECCP1AS;
extern volatile near struct {
  unsigned PSSBD0:1;
  unsigned PSSBD1:1;
  unsigned PSSAC0:1;
  unsigned PSSAC1:1;
  unsigned ECCPAS0:1;
  unsigned ECCPAS1:1;
  unsigned ECCPAS2:1;
  unsigned ECCPASE:1;
} ECCP1ASbits;
extern volatile near unsigned char       CCP1DEL;
extern volatile near struct {
  unsigned filler0:7;
  unsigned PRSEN:1;
} CCP1DELbits;
extern volatile near unsigned char       ECCP1DEL;
extern volatile near struct {
  unsigned PDC0:1;
  unsigned PDC1:1;
  unsigned PDC2:1;
  unsigned PDC3:1;
  unsigned PDC4:1;
  unsigned PDC5:1;
  unsigned PDC6:1;
  unsigned PRSEN:1;
} ECCP1DELbits;
extern volatile near unsigned char       BAUDCON;
extern volatile near union {
  struct {
    unsigned ABDEN:1;
    unsigned WUE:1;
    unsigned :1;
    unsigned BRG16:1;
    unsigned SCKP:1;
    unsigned :1;
    unsigned RCIDL:1;
    unsigned ABDOVF:1;
  };
  struct {
    unsigned :6;
    unsigned RCMT:1;
  };
} BAUDCONbits;
extern volatile near unsigned char       CCP2CON;
extern volatile near struct {
  unsigned CCP2M0:1;
  unsigned CCP2M1:1;
  unsigned CCP2M2:1;
  unsigned CCP2M3:1;
  unsigned DC2B0:1;
  unsigned DC2B1:1;
} CCP2CONbits;
extern volatile near unsigned            CCPR2;
extern volatile near unsigned char       CCPR2L;
extern volatile near unsigned char       CCPR2H;
extern volatile near unsigned char       CCP1CON;
extern volatile near struct {
  unsigned CCP1M0:1;
  unsigned CCP1M1:1;
  unsigned CCP1M2:1;
  unsigned CCP1M3:1;
  unsigned DC1B0:1;
  unsigned DC1B1:1;
  unsigned P1M0:1;
  unsigned P1M1:1;
} CCP1CONbits;
extern volatile near unsigned char       ECCP1CON;
extern volatile near struct {
  unsigned CCP1M0:1;
  unsigned CCP1M1:1;
  unsigned CCP1M2:1;
  unsigned CCP1M3:1;
  unsigned DC1B0:1;
  unsigned DC1B1:1;
  unsigned P1M0:1;
  unsigned P1M1:1;
} ECCP1CONbits;
extern volatile near unsigned            CCPR1;
extern volatile near unsigned char       CCPR1L;
extern volatile near unsigned char       CCPR1H;
extern volatile near unsigned char       ADCON2;
extern volatile near struct {
  unsigned ADCS0:1;
  unsigned ADCS1:1;
  unsigned ADCS2:1;
  unsigned ACQT0:1;
  unsigned ACQT1:1;
  unsigned ACQT2:1;
  unsigned :1;
  unsigned ADFM:1;
} ADCON2bits;
extern volatile near unsigned char       ADCON1;
extern volatile near struct {
  unsigned PCFG0:1;
  unsigned PCFG1:1;
  unsigned PCFG2:1;
  unsigned PCFG3:1;
  unsigned VCFG0:1;
  unsigned VCFG1:1;
} ADCON1bits;
extern volatile near unsigned char       ADCON0;
extern volatile near union {
  struct {
    unsigned ADON:1;
    unsigned GO_DONE:1;
    unsigned CHS0:1;
    unsigned CHS1:1;
    unsigned CHS2:1;
    unsigned CHS3:1;
  };
  struct {
    unsigned :1;
    unsigned DONE:1;
  };
  struct {
    unsigned :1;
    unsigned GO:1;
  };
  struct {
    unsigned :1;
    unsigned NOT_DONE:1;
  };
} ADCON0bits;
extern volatile near unsigned            ADRES;
extern volatile near unsigned char       ADRESL;
extern volatile near unsigned char       ADRESH;
extern volatile near unsigned char       SSPCON2;
extern volatile near struct {
  unsigned SEN:1;
  unsigned RSEN:1;
  unsigned PEN:1;
  unsigned RCEN:1;
  unsigned ACKEN:1;
  unsigned ACKDT:1;
  unsigned ACKSTAT:1;
  unsigned GCEN:1;
} SSPCON2bits;
extern volatile near unsigned char       SSPCON1;
extern volatile near struct {
  unsigned SSPM0:1;
  unsigned SSPM1:1;
  unsigned SSPM2:1;
  unsigned SSPM3:1;
  unsigned CKP:1;
  unsigned SSPEN:1;
  unsigned SSPOV:1;
  unsigned WCOL:1;
} SSPCON1bits;
extern volatile near unsigned char       SSPSTAT;
extern volatile near union {
  struct {
    unsigned BF:1;
    unsigned UA:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned CKE:1;
    unsigned SMP:1;
  };
  struct {
    unsigned :2;
    unsigned I2C_READ:1;
    unsigned I2C_START:1;
    unsigned I2C_STOP:1;
    unsigned I2C_DAT:1;
  };
  struct {
    unsigned :2;
    unsigned NOT_W:1;
    unsigned :2;
    unsigned NOT_A:1;
  };
  struct {
    unsigned :2;
    unsigned NOT_WRITE:1;
    unsigned :2;
    unsigned NOT_ADDRESS:1;
  };
  struct {
    unsigned :2;
    unsigned READ_WRITE:1;
    unsigned :2;
    unsigned DATA_ADDRESS:1;
  };
  struct {
    unsigned :2;
    unsigned R:1;
    unsigned :2;
    unsigned D:1;
  };
} SSPSTATbits;
extern volatile near unsigned char       SSPADD;
extern volatile near unsigned char       SSPBUF;
extern volatile near unsigned char       T2CON;
extern volatile near struct {
  unsigned T2CKPS0:1;
  unsigned T2CKPS1:1;
  unsigned TMR2ON:1;
  unsigned T2OUTPS0:1;
  unsigned T2OUTPS1:1;
  unsigned T2OUTPS2:1;
  unsigned T2OUTPS3:1;
} T2CONbits;
extern volatile near unsigned char       PR2;
extern volatile near unsigned char       TMR2;
extern volatile near unsigned char       T1CON;
extern volatile near union {
  struct {
    unsigned TMR1ON:1;
    unsigned TMR1CS:1;
    unsigned T1SYNC:1;
    unsigned T1OSCEN:1;
    unsigned T1CKPS0:1;
    unsigned T1CKPS1:1;
    unsigned T1RUN:1;
    unsigned RD16:1;
  };
  struct {
    unsigned :2;
    unsigned NOT_T1SYNC:1;
  };
} T1CONbits;
extern volatile near unsigned char       TMR1L;
extern volatile near unsigned char       TMR1H;
extern volatile near unsigned char       RCON;
extern volatile near union {
  struct {
    unsigned NOT_BOR:1;
    unsigned NOT_POR:1;
    unsigned NOT_PD:1;
    unsigned NOT_TO:1;
    unsigned NOT_RI:1;
    unsigned :1;
    unsigned SBOREN:1;
    unsigned NOT_IPEN:1;
  };
  struct {
    unsigned BOR:1;
    unsigned POR:1;
    unsigned PD:1;
    unsigned TO:1;
    unsigned RI:1;
    unsigned :2;
    unsigned IPEN:1;
  };
} RCONbits;
extern volatile near unsigned char       WDTCON;
extern volatile near union {
  struct {
    unsigned SWDTEN:1;
  };
  struct {
    unsigned SWDTE:1;
  };
} WDTCONbits;
extern volatile near unsigned char       HLVDCON;
extern volatile near union {
  struct {
    unsigned LVDL0:1;
    unsigned LVDL1:1;
    unsigned LVDL2:1;
    unsigned LVDL3:1;
    unsigned LVDEN:1;
    unsigned IRVST:1;
  };
  struct {
    unsigned LVV0:1;
    unsigned LVV1:1;
    unsigned LVV2:1;
    unsigned LVV3:1;
    unsigned :1;
    unsigned BGST:1;
  };
  struct {
    unsigned HLVDL0:1;
    unsigned HLVDL1:1;
    unsigned HLVDL2:1;
    unsigned HLVDL3:1;
    unsigned HLVDEN:1;
    unsigned :2;
    unsigned VDIRMAG:1;
  };
  struct {
    unsigned :5;
    unsigned IVRST:1;
  };
} HLVDCONbits;
extern volatile near unsigned char       LVDCON;
extern volatile near union {
  struct {
    unsigned LVDL0:1;
    unsigned LVDL1:1;
    unsigned LVDL2:1;
    unsigned LVDL3:1;
    unsigned LVDEN:1;
    unsigned IRVST:1;
  };
  struct {
    unsigned LVV0:1;
    unsigned LVV1:1;
    unsigned LVV2:1;
    unsigned LVV3:1;
    unsigned :1;
    unsigned BGST:1;
  };
  struct {
    unsigned HLVDL0:1;
    unsigned HLVDL1:1;
    unsigned HLVDL2:1;
    unsigned HLVDL3:1;
    unsigned HLVDEN:1;
    unsigned :2;
    unsigned VDIRMAG:1;
  };
  struct {
    unsigned :5;
    unsigned IVRST:1;
  };
} LVDCONbits;
extern volatile near unsigned char       OSCCON;
extern volatile near union {
  struct {
    unsigned SCS0:1;
    unsigned SCS1:1;
    unsigned IOFS:1;
    unsigned OSTS:1;
    unsigned IRCF0:1;
    unsigned IRCF1:1;
    unsigned IRCF2:1;
    unsigned IDLEN:1;
  };
  struct {
    unsigned :2;
    unsigned FLTS:1;
  };
} OSCCONbits;
extern volatile near unsigned char       T0CON;
extern volatile near struct {
  unsigned T0PS0:1;
  unsigned T0PS1:1;
  unsigned T0PS2:1;
  unsigned PSA:1;
  unsigned T0SE:1;
  unsigned T0CS:1;
  unsigned T08BIT:1;
  unsigned TMR0ON:1;
} T0CONbits;
extern volatile near unsigned char       TMR0L;
extern volatile near unsigned char       TMR0H;
extern          near unsigned char       STATUS;
extern          near struct {
  unsigned C:1;
  unsigned DC:1;
  unsigned Z:1;
  unsigned OV:1;
  unsigned N:1;
} STATUSbits;
extern          near unsigned            FSR2;
extern          near unsigned char       FSR2L;
extern          near unsigned char       FSR2H;
extern volatile near unsigned char       PLUSW2;
extern volatile near unsigned char       PREINC2;
extern volatile near unsigned char       POSTDEC2;
extern volatile near unsigned char       POSTINC2;
extern          near unsigned char       INDF2;
extern          near unsigned char       BSR;
extern          near unsigned            FSR1;
extern          near unsigned char       FSR1L;
extern          near unsigned char       FSR1H;
extern volatile near unsigned char       PLUSW1;
extern volatile near unsigned char       PREINC1;
extern volatile near unsigned char       POSTDEC1;
extern volatile near unsigned char       POSTINC1;
extern          near unsigned char       INDF1;
extern          near unsigned char       WREG;
extern          near unsigned            FSR0;
extern          near unsigned char       FSR0L;
extern          near unsigned char       FSR0H;
extern volatile near unsigned char       PLUSW0;
extern volatile near unsigned char       PREINC0;
extern volatile near unsigned char       POSTDEC0;
extern volatile near unsigned char       POSTINC0;
extern          near unsigned char       INDF0;
extern volatile near unsigned char       INTCON3;
extern volatile near union {
  struct {
    unsigned INT1IF:1;
    unsigned INT2IF:1;
    unsigned :1;
    unsigned INT1IE:1;
    unsigned INT2IE:1;
    unsigned :1;
    unsigned INT1IP:1;
    unsigned INT2IP:1;
  };
  struct {
    unsigned INT1F:1;
    unsigned INT2F:1;
    unsigned :1;
    unsigned INT1E:1;
    unsigned INT2E:1;
    unsigned :1;
    unsigned INT1P:1;
    unsigned INT2P:1;
  };
} INTCON3bits;
extern volatile near unsigned char       INTCON2;
extern volatile near union {
  struct {
    unsigned RBIP:1;
    unsigned :1;
    unsigned TMR0IP:1;
    unsigned :1;
    unsigned INTEDG2:1;
    unsigned INTEDG1:1;
    unsigned INTEDG0:1;
    unsigned NOT_RBPU:1;
  };
  struct {
    unsigned :2;
    unsigned T0IP:1;
    unsigned :4;
    unsigned RBPU:1;
  };
} INTCON2bits;
extern volatile near unsigned char       INTCON;
extern volatile near union {
  struct {
    unsigned RBIF:1;
    unsigned INT0IF:1;
    unsigned TMR0IF:1;
    unsigned RBIE:1;
    unsigned INT0IE:1;
    unsigned TMR0IE:1;
    unsigned PEIE:1;
    unsigned GIE:1;
  };
  struct {
    unsigned :1;
    unsigned INT0F:1;
    unsigned T0IF:1;
    unsigned :1;
    unsigned INT0E:1;
    unsigned T0IE:1;
    unsigned GIEL:1;
    unsigned GIEH:1;
  };
} INTCONbits;
extern          near unsigned            PROD;
extern          near unsigned char       PRODL;
extern          near unsigned char       PRODH;
extern volatile near unsigned char       TABLAT;
extern volatile near unsigned short long TBLPTR;
extern volatile near unsigned char       TBLPTRL;
extern volatile near unsigned char       TBLPTRH;
extern volatile near unsigned char       TBLPTRU;
extern volatile near unsigned short long PC;
extern volatile near unsigned char       PCL;
extern volatile near unsigned char       PCLATH;
extern volatile near unsigned char       PCLATU;
extern volatile near unsigned char       STKPTR;
extern volatile near struct {
  unsigned STKPTR0:1;
  unsigned STKPTR1:1;
  unsigned STKPTR2:1;
  unsigned STKPTR3:1;
  unsigned STKPTR4:1;
  unsigned :1;
  unsigned STKUNF:1;
  unsigned STKFUL:1;
} STKPTRbits;
extern          near unsigned short long TOS;
extern          near unsigned char       TOSL;
extern          near unsigned char       TOSH;
extern          near unsigned char       TOSU;



#line 1198 "C:/mcc18/h/p18f4550.h"
 
#line 1200 "C:/mcc18/h/p18f4550.h"
#line 1201 "C:/mcc18/h/p18f4550.h"


#line 1204 "C:/mcc18/h/p18f4550.h"
 
#line 1206 "C:/mcc18/h/p18f4550.h"
#line 1207 "C:/mcc18/h/p18f4550.h"
#line 1208 "C:/mcc18/h/p18f4550.h"
#line 1209 "C:/mcc18/h/p18f4550.h"

#line 1211 "C:/mcc18/h/p18f4550.h"
#line 1212 "C:/mcc18/h/p18f4550.h"
#line 1213 "C:/mcc18/h/p18f4550.h"
#line 1214 "C:/mcc18/h/p18f4550.h"
#line 1215 "C:/mcc18/h/p18f4550.h"


#line 1219 "C:/mcc18/h/p18f4550.h"
 
#line 1221 "C:/mcc18/h/p18f4550.h"


#line 1224 "C:/mcc18/h/p18f4550.h"
#line 139 "C:/mcc18/h/p18cxxx.h"

#line 141 "C:/mcc18/h/p18cxxx.h"
#line 143 "C:/mcc18/h/p18cxxx.h"
#line 145 "C:/mcc18/h/p18cxxx.h"
#line 147 "C:/mcc18/h/p18cxxx.h"
#line 149 "C:/mcc18/h/p18cxxx.h"
#line 151 "C:/mcc18/h/p18cxxx.h"
#line 153 "C:/mcc18/h/p18cxxx.h"
#line 155 "C:/mcc18/h/p18cxxx.h"
#line 157 "C:/mcc18/h/p18cxxx.h"
#line 159 "C:/mcc18/h/p18cxxx.h"
#line 161 "C:/mcc18/h/p18cxxx.h"
#line 163 "C:/mcc18/h/p18cxxx.h"
#line 165 "C:/mcc18/h/p18cxxx.h"
#line 167 "C:/mcc18/h/p18cxxx.h"
#line 169 "C:/mcc18/h/p18cxxx.h"
#line 171 "C:/mcc18/h/p18cxxx.h"
#line 173 "C:/mcc18/h/p18cxxx.h"
#line 175 "C:/mcc18/h/p18cxxx.h"
#line 177 "C:/mcc18/h/p18cxxx.h"
#line 179 "C:/mcc18/h/p18cxxx.h"
#line 181 "C:/mcc18/h/p18cxxx.h"
#line 183 "C:/mcc18/h/p18cxxx.h"
#line 185 "C:/mcc18/h/p18cxxx.h"
#line 187 "C:/mcc18/h/p18cxxx.h"
#line 189 "C:/mcc18/h/p18cxxx.h"
#line 191 "C:/mcc18/h/p18cxxx.h"
#line 193 "C:/mcc18/h/p18cxxx.h"
#line 195 "C:/mcc18/h/p18cxxx.h"
#line 197 "C:/mcc18/h/p18cxxx.h"
#line 199 "C:/mcc18/h/p18cxxx.h"

#line 201 "C:/mcc18/h/p18cxxx.h"
#line 38 "usb9.c"

#line 1 "./system\typedefs.h"

#line 35 "./system\typedefs.h"
 


#line 39 "./system\typedefs.h"

typedef unsigned char   byte;           
typedef unsigned int    word;           
typedef unsigned long   dword;          

typedef union _BYTE
{
    byte _byte;
    struct
    {
        unsigned b0:1;
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    };
} BYTE;

typedef union _WORD
{
    word _word;
    struct
    {
        byte byte0;
        byte byte1;
    };
    struct
    {
        BYTE Byte0;
        BYTE Byte1;
    };
    struct
    {
        BYTE LowB;
        BYTE HighB;
    };
    struct
    {
        byte v[2];
    };
} WORD;
#line 84 "./system\typedefs.h"
#line 85 "./system\typedefs.h"

typedef union _DWORD
{
    dword _dword;
    struct
    {
        byte byte0;
        byte byte1;
        byte byte2;
        byte byte3;
    };
    struct
    {
        word word0;
        word word1;
    };
    struct
    {
        BYTE Byte0;
        BYTE Byte1;
        BYTE Byte2;
        BYTE Byte3;
    };
    struct
    {
        WORD Word0;
        WORD Word1;
    };
    struct
    {
        byte v[4];
    };
} DWORD;
#line 119 "./system\typedefs.h"
#line 120 "./system\typedefs.h"
#line 121 "./system\typedefs.h"
#line 122 "./system\typedefs.h"

typedef void(*pFunc)(void);

typedef union _POINTER
{
    struct
    {
        byte bLow;
        byte bHigh;
        
    };
    word _word;                         
    
    

    byte* bRam;                         
                                        
    word* wRam;                         
                                        

    rom byte* bRom;                     
    rom word* wRom;
    
    
    
    
} POINTER;

typedef enum _BOOL { FALSE = 0, TRUE } BOOL;

#line 153 "./system\typedefs.h"
#line 154 "./system\typedefs.h"

#line 156 "./system\typedefs.h"
#line 39 "usb9.c"

#line 1 "./system\usb\usb.h"

#line 35 "./system\usb\usb.h"
 

#line 38 "./system\usb\usb.h"


#line 45 "./system\usb\usb.h"
 

#line 1 "./autofiles\usbcfg.h"

#line 32 "./autofiles\usbcfg.h"
 


#line 36 "./autofiles\usbcfg.h"

 
#line 39 "./autofiles\usbcfg.h"
#line 40 "./autofiles\usbcfg.h"

 
#line 43 "./autofiles\usbcfg.h"
#line 44 "./autofiles\usbcfg.h"

#line 46 "./autofiles\usbcfg.h"
#line 47 "./autofiles\usbcfg.h"

 
#line 50 "./autofiles\usbcfg.h"


#line 55 "./autofiles\usbcfg.h"
 
#line 57 "./autofiles\usbcfg.h"
#line 58 "./autofiles\usbcfg.h"
#line 59 "./autofiles\usbcfg.h"
#line 60 "./autofiles\usbcfg.h"
#line 61 "./autofiles\usbcfg.h"

 

#line 65 "./autofiles\usbcfg.h"
 
 
 
#line 69 "./autofiles\usbcfg.h"
#line 70 "./autofiles\usbcfg.h"
#line 71 "./autofiles\usbcfg.h"
#line 72 "./autofiles\usbcfg.h"
#line 73 "./autofiles\usbcfg.h"
#line 74 "./autofiles\usbcfg.h"
#line 75 "./autofiles\usbcfg.h"


 

#line 87 "./autofiles\usbcfg.h"
 
 


#line 94 "./autofiles\usbcfg.h"
#line 95 "./autofiles\usbcfg.h"


#line 101 "./autofiles\usbcfg.h"

#line 107 "./autofiles\usbcfg.h"

#line 108 "./autofiles\usbcfg.h"
 

#line 111 "./autofiles\usbcfg.h"

#line 113 "./autofiles\usbcfg.h"
#line 47 "./system\usb\usb.h"

#line 1 "./system\usb\usbdefs\usbdefs_std_dsc.h"

#line 35 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 


#line 39 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 

#line 42 "./system\usb\usbdefs\usbdefs_std_dsc.h"

 
#line 1 "./system\typedefs.h"

#line 35 "./system\typedefs.h"
 

#line 156 "./system\typedefs.h"
#line 44 "./system\usb\usbdefs\usbdefs_std_dsc.h"


 

 
#line 50 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 51 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 52 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 53 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 54 "./system\usb\usbdefs\usbdefs_std_dsc.h"


#line 62 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 
#line 64 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 65 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 66 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 67 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 68 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 69 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 70 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 71 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 72 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 73 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 74 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 75 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 76 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 77 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 78 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 79 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 80 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 81 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 82 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 83 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 84 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 85 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 86 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 87 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 88 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 89 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 90 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 91 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 92 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 93 "./system\usb\usbdefs\usbdefs_std_dsc.h"

 
#line 96 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 97 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 98 "./system\usb\usbdefs\usbdefs_std_dsc.h"

 
#line 101 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 102 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 103 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 104 "./system\usb\usbdefs\usbdefs_std_dsc.h"

 
#line 107 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 108 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 109 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 110 "./system\usb\usbdefs\usbdefs_std_dsc.h"

 
#line 113 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 114 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 115 "./system\usb\usbdefs\usbdefs_std_dsc.h"


 


#line 121 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 
typedef struct _USB_DEV_DSC
{
    byte bLength;       byte bDscType;      word bcdUSB;
    byte bDevCls;       byte bDevSubCls;    byte bDevProtocol;
    byte bMaxPktSize0;  word idVendor;      word idProduct;
    word bcdDevice;     byte iMFR;          byte iProduct;
    byte iSerialNum;    byte bNumCfg;
} USB_DEV_DSC;


#line 133 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 
typedef struct _USB_CFG_DSC
{
    byte bLength;       byte bDscType;      word wTotalLength;
    byte bNumIntf;      byte bCfgValue;     byte iCfg;
    byte bmAttributes;  byte bMaxPower;
} USB_CFG_DSC;


#line 143 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 
typedef struct _USB_INTF_DSC
{
    byte bLength;       byte bDscType;      byte bIntfNum;
    byte bAltSetting;   byte bNumEPs;       byte bIntfCls;
    byte bIntfSubCls;   byte bIntfProtocol; byte iIntf;
} USB_INTF_DSC;


#line 153 "./system\usb\usbdefs\usbdefs_std_dsc.h"
 
typedef struct _USB_EP_DSC
{
    byte bLength;       byte bDscType;      byte bEPAdr;
    byte bmAttributes;  word wMaxPktSize;   byte bInterval;
} USB_EP_DSC;

#line 161 "./system\usb\usbdefs\usbdefs_std_dsc.h"
#line 48 "./system\usb\usb.h"

#line 1 "./autofiles\usbdsc.h"

#line 32 "./autofiles\usbdsc.h"
 


#line 37 "./autofiles\usbdsc.h"
 


#line 41 "./autofiles\usbdsc.h"

 
#line 1 "./system\typedefs.h"

#line 35 "./system\typedefs.h"
 

#line 156 "./system\typedefs.h"
#line 43 "./autofiles\usbdsc.h"

#line 1 "./autofiles\usbcfg.h"

#line 32 "./autofiles\usbcfg.h"
 


#line 55 "./autofiles\usbcfg.h"

#line 65 "./autofiles\usbcfg.h"

#line 87 "./autofiles\usbcfg.h"

#line 94 "./autofiles\usbcfg.h"

#line 101 "./autofiles\usbcfg.h"

#line 107 "./autofiles\usbcfg.h"

#line 108 "./autofiles\usbcfg.h"
#line 113 "./autofiles\usbcfg.h"
#line 44 "./autofiles\usbdsc.h"


#line 47 "./autofiles\usbdsc.h"
#line 49 "./autofiles\usbdsc.h"

#line 51 "./autofiles\usbdsc.h"

#line 53 "./autofiles\usbdsc.h"

#line 1 "./system\usb\usb.h"

#line 35 "./system\usb\usb.h"
 

#line 45 "./system\usb\usb.h"
#line 59 "./system\usb\usb.h"
#line 61 "./system\usb\usb.h"
#line 62 "./system\usb\usb.h"
#line 64 "./system\usb\usb.h"
#line 65 "./system\usb\usb.h"
#line 54 "./autofiles\usbdsc.h"


 

#line 62 "./autofiles\usbdsc.h"
#line 63 "./autofiles\usbdsc.h"

 
extern rom USB_DEV_DSC device_dsc;
extern rom struct { USB_CFG_DSC cd01; USB_INTF_DSC i00a00; USB_MSD_DSC msd_i00a00; USB_EP_DSC ep01i_i00a00; } cfg01 ;
extern rom const unsigned char *rom USB_CD_Ptr[];
extern rom const unsigned char *rom USB_SD_Ptr[];


extern rom pFunc ClassReqHandler[1];

#line 74 "./autofiles\usbdsc.h"
#line 49 "./system\usb\usb.h"


#line 1 "./system\usb\usbdefs\usbdefs_ep0_buff.h"

#line 35 "./system\usb\usbdefs\usbdefs_ep0_buff.h"
 


#line 39 "./system\usb\usbdefs\usbdefs_ep0_buff.h"
 

#line 42 "./system\usb\usbdefs\usbdefs_ep0_buff.h"

 
#line 1 "./system\typedefs.h"

#line 35 "./system\typedefs.h"
 

#line 156 "./system\typedefs.h"
#line 44 "./system\usb\usbdefs\usbdefs_ep0_buff.h"

#line 1 "./autofiles\usbcfg.h"

#line 32 "./autofiles\usbcfg.h"
 


#line 55 "./autofiles\usbcfg.h"

#line 65 "./autofiles\usbcfg.h"

#line 87 "./autofiles\usbcfg.h"

#line 94 "./autofiles\usbcfg.h"

#line 101 "./autofiles\usbcfg.h"

#line 107 "./autofiles\usbcfg.h"

#line 108 "./autofiles\usbcfg.h"
#line 113 "./autofiles\usbcfg.h"
#line 45 "./system\usb\usbdefs\usbdefs_ep0_buff.h"
       


#line 58 "./system\usb\usbdefs\usbdefs_ep0_buff.h"
 
typedef union _CTRL_TRF_SETUP
{
     
    struct
    {
        byte _byte[8 ];
    };
    
     
    struct
    {
        byte bmRequestType;
        byte bRequest;    
        word wValue;
        word wIndex;
        word wLength;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        WORD W_Value;
        WORD W_Index;
        WORD W_Length;
    };
    struct
    {
        unsigned Recipient:5;           
        unsigned RequestType:2;         
        unsigned DataDir:1;             
        unsigned :8;
        byte bFeature;                  
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        byte bDscIndex;                 
        byte bDscType;                  
        word wLangID;                   
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        BYTE bDevADR;                   
        byte bDevADRH;                  
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        byte bCfgValue;                 
        byte bCfgRSD;                   
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        byte bAltID;                    
        byte bAltID_H;                  
        byte bIntfID;                   
        byte bIntfID_H;                 
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        byte bEPID;                     
        byte bEPID_H;                   
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned EPNum:4;               
        unsigned :3;
        unsigned EPDir:1;               
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
     
    
} CTRL_TRF_SETUP;


#line 178 "./system\usb\usbdefs\usbdefs_ep0_buff.h"
 
typedef union _CTRL_TRF_DATA
{
     
    struct
    {
        byte _byte[8 ];
    };
    
     
    struct
    {
        byte _byte0;
        byte _byte1;
        byte _byte2;
        byte _byte3;
        byte _byte4;
        byte _byte5;
        byte _byte6;
        byte _byte7;
    };
    struct
    {
        word _word0;
        word _word1;
        word _word2;
        word _word3;
    };

} CTRL_TRF_DATA;

#line 210 "./system\usb\usbdefs\usbdefs_ep0_buff.h"
#line 51 "./system\usb\usb.h"

#line 1 "./system\usb\usbmmap.h"

#line 35 "./system\usb\usbmmap.h"
 


#line 39 "./system\usb\usbmmap.h"

 
#line 1 "./system\typedefs.h"

#line 35 "./system\typedefs.h"
 

#line 156 "./system\typedefs.h"
#line 41 "./system\usb\usbmmap.h"


 

 
#line 47 "./system\usb\usbmmap.h"
#line 48 "./system\usb\usbmmap.h"
#line 49 "./system\usb\usbmmap.h"
#line 50 "./system\usb\usbmmap.h"
#line 51 "./system\usb\usbmmap.h"
#line 52 "./system\usb\usbmmap.h"
#line 53 "./system\usb\usbmmap.h"
#line 54 "./system\usb\usbmmap.h"
#line 55 "./system\usb\usbmmap.h"

 
#line 58 "./system\usb\usbmmap.h"
#line 59 "./system\usb\usbmmap.h"
#line 60 "./system\usb\usbmmap.h"
#line 61 "./system\usb\usbmmap.h"
#line 62 "./system\usb\usbmmap.h"
#line 63 "./system\usb\usbmmap.h"
#line 64 "./system\usb\usbmmap.h"

 
#line 67 "./system\usb\usbmmap.h"
#line 68 "./system\usb\usbmmap.h"

 
typedef union _USB_DEVICE_STATUS
{
    byte _byte;
    struct
    {
        unsigned RemoteWakeup:1;
        unsigned ctrl_trf_mem:1;
    };
} USB_DEVICE_STATUS;

typedef union _BD_STAT
{
    byte _byte;
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned BSTALL:1;              
        unsigned DTSEN:1;               
        unsigned INCDIS:1;              
        unsigned KEN:1;                 
        unsigned DTS:1;                 
        unsigned UOWN:1;                
    };
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
        unsigned :1;
        unsigned UOWN:1;
    };
    struct{
        unsigned :2;
        unsigned PID:4;                 
        unsigned :2;
    };
} BD_STAT;                              

typedef union _BDT
{
    struct
    {
        BD_STAT Stat;
        byte Cnt;
        byte ADRL;                      
        byte ADRH;                      
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        byte* ADR;                      
    };
} BDT;                                  

 
extern byte usb_device_state;
extern USB_DEVICE_STATUS usb_stat;
extern byte usb_active_cfg;
extern byte usb_alt_intf[1 ];

extern volatile far BDT ep0Bo;          
extern volatile far BDT ep0Bi;          
extern volatile far BDT ep1Bo;          
extern volatile far BDT ep1Bi;          
extern volatile far BDT ep2Bo;          
extern volatile far BDT ep2Bi;          
extern volatile far BDT ep3Bo;          
extern volatile far BDT ep3Bi;          
extern volatile far BDT ep4Bo;          
extern volatile far BDT ep4Bi;          
extern volatile far BDT ep5Bo;          
extern volatile far BDT ep5Bi;          
extern volatile far BDT ep6Bo;          
extern volatile far BDT ep6Bi;          
extern volatile far BDT ep7Bo;          
extern volatile far BDT ep7Bi;          
extern volatile far BDT ep8Bo;          
extern volatile far BDT ep8Bi;          
extern volatile far BDT ep9Bo;          
extern volatile far BDT ep9Bi;          
extern volatile far BDT ep10Bo;         
extern volatile far BDT ep10Bi;         
extern volatile far BDT ep11Bo;         
extern volatile far BDT ep11Bi;         
extern volatile far BDT ep12Bo;         
extern volatile far BDT ep12Bi;         
extern volatile far BDT ep13Bo;         
extern volatile far BDT ep13Bi;         
extern volatile far BDT ep14Bo;         
extern volatile far BDT ep14Bi;         
extern volatile far BDT ep15Bo;         
extern volatile far BDT ep15Bi;         

extern volatile far CTRL_TRF_SETUP SetupPkt;
extern volatile far CTRL_TRF_DATA CtrlTrfData;

#line 173 "./system\usb\usbmmap.h"
 
#line 175 "./system\usb\usbmmap.h"
extern volatile far unsigned char msd_report_out[64 ];
extern volatile far unsigned char msd_report_in[64 ]; 
#line 178 "./system\usb\usbmmap.h"

#line 180 "./system\usb\usbmmap.h"
#line 52 "./system\usb\usb.h"


#line 1 "./system\usb\usbdrv\usbdrv.h"

#line 35 "./system\usb\usbdrv\usbdrv.h"
 


#line 39 "./system\usb\usbdrv\usbdrv.h"

 
#line 1 "./system\typedefs.h"

#line 35 "./system\typedefs.h"
 

#line 156 "./system\typedefs.h"
#line 41 "./system\usb\usbdrv\usbdrv.h"

#line 1 "./system\usb\usb.h"

#line 35 "./system\usb\usb.h"
 

#line 45 "./system\usb\usb.h"
#line 59 "./system\usb\usb.h"
#line 61 "./system\usb\usb.h"
#line 62 "./system\usb\usb.h"
#line 64 "./system\usb\usb.h"
#line 65 "./system\usb\usb.h"
#line 42 "./system\usb\usbdrv\usbdrv.h"


 

 
#line 48 "./system\usb\usbdrv\usbdrv.h"
#line 49 "./system\usb\usbdrv\usbdrv.h"
#line 50 "./system\usb\usbdrv\usbdrv.h"
#line 51 "./system\usb\usbdrv\usbdrv.h"
#line 52 "./system\usb\usbdrv\usbdrv.h"
#line 53 "./system\usb\usbdrv\usbdrv.h"
#line 54 "./system\usb\usbdrv\usbdrv.h"
#line 55 "./system\usb\usbdrv\usbdrv.h"
#line 56 "./system\usb\usbdrv\usbdrv.h"
#line 57 "./system\usb\usbdrv\usbdrv.h"

 
#line 60 "./system\usb\usbdrv\usbdrv.h"
#line 61 "./system\usb\usbdrv\usbdrv.h"
#line 62 "./system\usb\usbdrv\usbdrv.h"
#line 63 "./system\usb\usbdrv\usbdrv.h"
#line 64 "./system\usb\usbdrv\usbdrv.h"
                                    


#line 74 "./system\usb\usbdrv\usbdrv.h"
 
#line 76 "./system\usb\usbdrv\usbdrv.h"
#line 77 "./system\usb\usbdrv\usbdrv.h"

#line 79 "./system\usb\usbdrv\usbdrv.h"
#line 80 "./system\usb\usbdrv\usbdrv.h"

#line 82 "./system\usb\usbdrv\usbdrv.h"
#line 83 "./system\usb\usbdrv\usbdrv.h"
#line 84 "./system\usb\usbdrv\usbdrv.h"
#line 85 "./system\usb\usbdrv\usbdrv.h"
#line 86 "./system\usb\usbdrv\usbdrv.h"
#line 87 "./system\usb\usbdrv\usbdrv.h"
#line 88 "./system\usb\usbdrv\usbdrv.h"
#line 89 "./system\usb\usbdrv\usbdrv.h"
#line 90 "./system\usb\usbdrv\usbdrv.h"
#line 91 "./system\usb\usbdrv\usbdrv.h"
#line 92 "./system\usb\usbdrv\usbdrv.h"
#line 93 "./system\usb\usbdrv\usbdrv.h"
#line 94 "./system\usb\usbdrv\usbdrv.h"
#line 95 "./system\usb\usbdrv\usbdrv.h"
#line 96 "./system\usb\usbdrv\usbdrv.h"
#line 97 "./system\usb\usbdrv\usbdrv.h"
#line 98 "./system\usb\usbdrv\usbdrv.h"
#line 99 "./system\usb\usbdrv\usbdrv.h"
#line 100 "./system\usb\usbdrv\usbdrv.h"
#line 101 "./system\usb\usbdrv\usbdrv.h"
#line 102 "./system\usb\usbdrv\usbdrv.h"
#line 103 "./system\usb\usbdrv\usbdrv.h"
#line 104 "./system\usb\usbdrv\usbdrv.h"
#line 105 "./system\usb\usbdrv\usbdrv.h"
#line 106 "./system\usb\usbdrv\usbdrv.h"
#line 107 "./system\usb\usbdrv\usbdrv.h"
#line 108 "./system\usb\usbdrv\usbdrv.h"
#line 109 "./system\usb\usbdrv\usbdrv.h"
#line 110 "./system\usb\usbdrv\usbdrv.h"
#line 111 "./system\usb\usbdrv\usbdrv.h"
#line 112 "./system\usb\usbdrv\usbdrv.h"
#line 113 "./system\usb\usbdrv\usbdrv.h"


#line 134 "./system\usb\usbdrv\usbdrv.h"
 

#line 139 "./system\usb\usbdrv\usbdrv.h"
#line 140 "./system\usb\usbdrv\usbdrv.h"


#line 157 "./system\usb\usbdrv\usbdrv.h"
 
#line 159 "./system\usb\usbdrv\usbdrv.h"

#line 163 "./system\usb\usbdrv\usbdrv.h"

#line 164 "./system\usb\usbdrv\usbdrv.h"
 


#line 188 "./system\usb\usbdrv\usbdrv.h"
 

#line 194 "./system\usb\usbdrv\usbdrv.h"
#line 195 "./system\usb\usbdrv\usbdrv.h"

 

 

 
void USBCheckBusStatus(void);
void USBDriverService(void);
void USBRemoteWakeup(void);
void USBSoftDetach(void); 

void ClearArray(byte* startAdr,byte count);
#line 208 "./system\usb\usbdrv\usbdrv.h"
#line 54 "./system\usb\usb.h"




#line 59 "./system\usb\usb.h"
#line 61 "./system\usb\usb.h"
#line 62 "./system\usb\usb.h"

#line 64 "./system\usb\usb.h"
#line 65 "./system\usb\usb.h"
#line 40 "usb9.c"

#line 1 "./io_cfg.h"

#line 35 "./io_cfg.h"
 


#line 39 "./io_cfg.h"

 
#line 1 "./autofiles\usbcfg.h"

#line 32 "./autofiles\usbcfg.h"
 


#line 55 "./autofiles\usbcfg.h"

#line 65 "./autofiles\usbcfg.h"

#line 87 "./autofiles\usbcfg.h"

#line 94 "./autofiles\usbcfg.h"

#line 101 "./autofiles\usbcfg.h"

#line 107 "./autofiles\usbcfg.h"

#line 108 "./autofiles\usbcfg.h"
#line 113 "./autofiles\usbcfg.h"
#line 41 "./io_cfg.h"


 
#line 45 "./io_cfg.h"
#line 46 "./io_cfg.h"

 
#line 49 "./io_cfg.h"

#line 51 "./io_cfg.h"
#line 52 "./io_cfg.h"
#line 53 "./io_cfg.h"
#line 55 "./io_cfg.h"

#line 57 "./io_cfg.h"

#line 59 "./io_cfg.h"
#line 60 "./io_cfg.h"
#line 61 "./io_cfg.h"
#line 63 "./io_cfg.h"


#line 66 "./io_cfg.h"
#line 67 "./io_cfg.h"
#line 68 "./io_cfg.h"
#line 69 "./io_cfg.h"
#line 70 "./io_cfg.h"
#line 71 "./io_cfg.h"

#line 73 "./io_cfg.h"

 
#line 76 "./io_cfg.h"

#line 78 "./io_cfg.h"
#line 79 "./io_cfg.h"
#line 80 "./io_cfg.h"
#line 81 "./io_cfg.h"

#line 83 "./io_cfg.h"
#line 84 "./io_cfg.h"
#line 85 "./io_cfg.h"
#line 86 "./io_cfg.h"

#line 88 "./io_cfg.h"
#line 89 "./io_cfg.h"
#line 90 "./io_cfg.h"
#line 91 "./io_cfg.h"

#line 93 "./io_cfg.h"
#line 94 "./io_cfg.h"
#line 95 "./io_cfg.h"
#line 96 "./io_cfg.h"

 
#line 99 "./io_cfg.h"
#line 100 "./io_cfg.h"
#line 101 "./io_cfg.h"
#line 102 "./io_cfg.h"
#line 103 "./io_cfg.h"

 
#line 106 "./io_cfg.h"

 
#line 109 "./io_cfg.h"
#line 110 "./io_cfg.h"

#line 112 "./io_cfg.h"
#line 113 "./io_cfg.h"

 
#line 116 "./io_cfg.h"
#line 117 "./io_cfg.h"

#line 119 "./io_cfg.h"
#line 120 "./io_cfg.h"

#line 122 "./io_cfg.h"
#line 41 "usb9.c"
                     

 
#pragma udata

 
void USBStdGetDscHandler(void);
void USBStdSetCfgHandler(void);
void USBStdGetStatusHandler(void);
void USBStdFeatureReqHandler(void);

 
#pragma code

#line 69 "usb9.c"
 
void USBCheckStdRequest(void)
{   
    if(SetupPkt.RequestType != STANDARD) return;
    
    switch(SetupPkt.bRequest)
    {
        case SET_ADR:
            ctrl_trf_session_owner = 1 ;
            usb_device_state = 4 ;       
             
            break;
        case GET_DSC:
            USBStdGetDscHandler();
            break;
        case SET_CFG:
            USBStdSetCfgHandler();
            break;
        case GET_CFG:
            ctrl_trf_session_owner = 1 ;
            pSrc.bRam = (byte*)&usb_active_cfg;         
            usb_stat.ctrl_trf_mem = 0 ;               
            ((wCount).v[0])  = 1;                            
            break;
        case GET_STATUS:
            USBStdGetStatusHandler();
            break;
        case CLR_FEATURE:
        case SET_FEATURE:
            USBStdFeatureReqHandler();
            break;
        case GET_INTF:
            ctrl_trf_session_owner = 1 ;
            pSrc.bRam = (byte*)&usb_alt_intf+SetupPkt.bIntfID;  
            usb_stat.ctrl_trf_mem = 0 ;               
            ((wCount).v[0])  = 1;                            
            break;
        case SET_INTF:
            ctrl_trf_session_owner = 1 ;
            usb_alt_intf[SetupPkt.bIntfID] = SetupPkt.bAltID;
            break;
        case SET_DSC:
        case SYNCH_FRAME:
        default:
            break;
    }
    
}


#line 135 "usb9.c"
 
void USBStdGetDscHandler(void)
{
    if(SetupPkt.bmRequestType == 0x80)
    {
        switch(SetupPkt.bDscType)
        {
            case 0x01 :
                ctrl_trf_session_owner = 1 ;
                pSrc.bRom = (rom byte*)&device_dsc;
                wCount._word = sizeof(device_dsc);          
                break;
            case 0x02 :
                ctrl_trf_session_owner = 1 ;
                pSrc.bRom = *(USB_CD_Ptr+SetupPkt.bDscIndex);
                wCount._word = *(pSrc.wRom+1);              
                break;
            case 0x03 :
                ctrl_trf_session_owner = 1 ;
                pSrc.bRom = *(USB_SD_Ptr+SetupPkt.bDscIndex);
                wCount._word = *pSrc.bRom;                  
                break;
        }
        
        usb_stat.ctrl_trf_mem = 1 ;                       
    }
}


#line 179 "usb9.c"
 
void USBStdSetCfgHandler(void)
{
    ctrl_trf_session_owner = 1 ;
    ClearArray((byte*)&UEP1,15); ;                          
    ClearArray((byte*)&usb_alt_intf,1 );
    usb_active_cfg = SetupPkt.bCfgValue;
    if(SetupPkt.bCfgValue == 0)
        usb_device_state = 5 ;
    else
    {
        usb_device_state = 6 ;

         
        
#line 195 "usb9.c"
#line 197 "usb9.c"
        
         

    }
}


#line 217 "usb9.c"
 
void USBStdGetStatusHandler(void)
{
    CtrlTrfData._byte0 = 0;                         
    CtrlTrfData._byte1 = 0;
        
    switch(SetupPkt.Recipient)
    {
        case RCPT_DEV:
            ctrl_trf_session_owner = 1 ;
            
#line 230 "usb9.c"
 
            if(PORTAbits.RA2  == 1)                     
                CtrlTrfData._byte0|=0b000000001;    
            
            if(usb_stat.RemoteWakeup == 1)          
                CtrlTrfData._byte0|=0b00000010;     
            break;
        case RCPT_INTF:
            ctrl_trf_session_owner = 1 ;     
            break;
        case RCPT_EP:
            ctrl_trf_session_owner = 1 ;
            
#line 244 "usb9.c"
 
            pDst.bRam = (byte*)&ep0Bo+(SetupPkt.EPNum*8)+(SetupPkt.EPDir*4);
            if(*pDst.bRam & 0x04 )    
                CtrlTrfData._byte0=0x01;
            break;
    }
    
    if(ctrl_trf_session_owner == 1 )
    {
        pSrc.bRam = (byte*)&CtrlTrfData;            
        usb_stat.ctrl_trf_mem = 0 ;               
        ((wCount).v[0])  = 2;                            
    }
}


#line 274 "usb9.c"
 
void USBStdFeatureReqHandler(void)
{
    if((SetupPkt.bFeature == DEVICE_REMOTE_WAKEUP)&&
       (SetupPkt.Recipient == RCPT_DEV))
    {
        ctrl_trf_session_owner = 1 ;
        if(SetupPkt.bRequest == SET_FEATURE)
            usb_stat.RemoteWakeup = 1;
        else
            usb_stat.RemoteWakeup = 0;
    }
    
    if((SetupPkt.bFeature == ENDPOINT_HALT)&&
       (SetupPkt.Recipient == RCPT_EP)&&
       (SetupPkt.EPNum != 0))
    {
        ctrl_trf_session_owner = 1 ;
         
        pDst.bRam = (byte*)&ep0Bo+(SetupPkt.EPNum*8)+(SetupPkt.EPDir*4);
        
        if(SetupPkt.bRequest == SET_FEATURE)
            *pDst.bRam = 0x80 |0x04 ;
        else
        {
            if(SetupPkt.EPDir == 1) 
                *pDst.bRam = 0x00 ;
            else
                *pDst.bRam = 0x80 |0x00 |0x08 ;
        }
    }
}

 
