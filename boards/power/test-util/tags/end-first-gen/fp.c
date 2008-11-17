#include <stdio.h>

#define u8 unsigned char

typedef struct {
    u8 bytestoreadin;
    void (*docmd)(u8 *data);
} t_command;


void setleds(u8 *data){
    printf("Setting leds to %x\n", data[0]);
}

void beefoutusb(u8 *data){
    printf("Sending %x and 31 others out of the USB\n", data[0]);
}

t_command commands[] = {{1, &setleds},
                        {32, &beefoutusb}};

int main(int argc, char *argv[]){
    u8 receiveddata[32];
    receiveddata = 5;
    commands[0].docmd(receiveddata);
    receiveddata = 6;
    commands[1].docmd(receiveddata);

    u8 curcmd = 1;
    u8 datapos = 0;
    
    while(datapos < commands[curcmd].bytestoreadin)
        receiveddata[datapos++] = getbytefromi2c();
    
    commands[curcmd].docmd(receiveddata);
}
