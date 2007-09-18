#include "musart.h"
#include <p18cxxx.h>
#include "system\usb\usb.h"
#pragma udata usart_section

unsigned char usart_in_buffer[USART_IN_BUFFER_SIZE];
unsigned char usart_in_head = 0; //Pointer to where to put
								 //the next byte
unsigned char usart_in_tail = 0; //pointer to the next byte to process
unsigned char usart_send_buffer[USART_SEND_BUFFER_SIZE];
unsigned char usart_send_head = 0; //Pointer to where to put
								   //the next byte
unsigned char usart_send_tail = 0; //Pointer to next byte to send
unsigned char dump;

unsigned char usart_data_waiting(){
	if(usart_in_head != usart_in_tail)
		return 1;
	else
		return 0;
}

void manage_usart(){
	//Run once per cycle to send and receive serial data
	//If there the tail in the send buffer trails the head
	//then send the data pointed to by the tail and increment
	//the tail
	if(usart_send_tail != usart_send_head)
		//Data to send
		if(PIR1bits.TXIF){
    		TXREG = usart_send_buffer[usart_send_tail++];
    		//Loop the tail pointer
	    	if(usart_send_tail == USART_SEND_BUFFER_SIZE)
	    		usart_send_tail = 0;
	    }
    
    //If there is a byte to receive, grab it and put it into the buffer
    //If the head of the buffer is one before the tail then
    //dump the byte
    if(PIR1bits.RCIF)
    	if((usart_in_head == (usart_in_tail - 1)) ||
    	   ((usart_in_tail == 0) && (usart_in_head == (USART_IN_BUFFER_SIZE - 1)))){
    	   	//Need to dump data. Buffer full
    	   	dump = RCREG;
    	} else {
    		usart_in_buffer[usart_in_head++] = RCREG;
    		//PIR1bits.RCIF = 0;
    		if(usart_in_head == USART_IN_BUFFER_SIZE)
    			usart_in_head = 0;
    	}
}

void flush_usart_send(){
	while(usart_send_tail != usart_send_head)
		//Data to send
		if(PIR1bits.TXIF){
    		TXREG = usart_send_buffer[usart_send_tail++];
    		//Loop the tail pointer
	    	if(usart_send_tail == USART_SEND_BUFFER_SIZE)
	    		usart_send_tail = 0;
	    }
}

void mputcharUSART(unsigned char data){
   usart_send_buffer[usart_send_head++] = data;
   if(usart_send_head == USART_SEND_BUFFER_SIZE)
	   usart_send_head = 0;
}

void mputrstringUSART(const rom unsigned char *data){
	unsigned char len = 0;
	while(data[len]){
		usart_send_buffer[usart_send_head++] = data[len++];
	   	if(usart_send_head == USART_SEND_BUFFER_SIZE)
		   	usart_send_head = 0;
	}
}

void mputlenstringUSART(unsigned char *data, unsigned char len){
	unsigned char i;
	for(i=0;i<len;i++){
		usart_send_buffer[usart_send_head++] = data[i];
	   	if(usart_send_head == USART_SEND_BUFFER_SIZE)
		   	usart_send_head = 0;
	}
}

unsigned char space_in_send_buffer(unsigned char len){
	if(usart_send_head < usart_send_tail)
		if((usart_in_tail - usart_in_head) > len)
			return 1;
		else
			return 0;
	else
		if((USART_SEND_BUFFER_SIZE - (usart_in_head - usart_in_tail)) > len)
			return 1;
		else
			return 0;
}

unsigned char mgetsUSART(unsigned char *buffer, unsigned char len){
	unsigned char lenavailable;
	unsigned char pos;
	
	if(usart_in_head == usart_in_tail)
		return 0;

	if(usart_in_head > usart_in_tail)
		lenavailable = usart_in_head - usart_in_tail;
	else
		lenavailable = USART_IN_BUFFER_SIZE - (usart_in_tail - usart_in_head);
		
	if(lenavailable < len)
		len = lenavailable;
	
	for(pos = 0; pos < len; pos++){
		buffer[pos] = usart_in_buffer[usart_in_tail++];
		if(usart_in_tail == USART_IN_BUFFER_SIZE)
			usart_in_tail = 0;
	}
	
	return len;
}
