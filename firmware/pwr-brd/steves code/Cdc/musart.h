#ifndef USART
#define USART
#pragma udata usart_section
#define USART_IN_BUFFER_SIZE 6
#define USART_SEND_BUFFER_SIZE 32

void manage_usart( void );

void init_usart(void);

void mputcharUSART(unsigned char data);

void mputrstringUSART(const rom unsigned char *data);

unsigned char mgetsUSART(unsigned char *buffer, unsigned char len);

unsigned char mUSARTIsTxTrfReady(void);

void mputlenstringUSART(unsigned char *data, unsigned char len);

unsigned char space_in_send_buffer(unsigned char len);

void flush_usart_send( void );
#endif
