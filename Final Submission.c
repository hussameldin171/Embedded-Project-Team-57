#include "stdint.h"
#include "tm4c123gh6pm.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#define r 6371*1000
#define pi 3.14159265

void SystemInit(){}

void milli_delay(int n) {
    int i , j ;
for( i=0;i<n;i++) {
for(j=0;j<3180;j++){}
}
}

/**UART,GPS and Conversion Methods***/


char latitude[50]  ; char longitude[50]; //global values to store parsed data



void UART2_Init() {
    // Initializing UART2 in PORT D
    //PD6 --> Rx
    //PD7 --> Tx

    SYSCTL_RCGCUART_R |= 0x04 ; //enable UART2 Clock
    SYSCTL_RCGCGPIO_R |= 0x08 ;
    while( (SYSCTL_PRGPIO_R & 0x08) == 0 ) {}

    UART2_CTL_R  &= 0xFFFE; //Enable = 0
    UART2_IBRD_R = 104 ; //setting baud rate tp 9600
    UART2_FBRD_R = 11 ;
    UART2_LCRH_R = 0x70 ; //fifo enabled, 8 bit word length , one stop bit , no  parity check
    UART2_CTL_R  |= 0x0201; //Enable = 1 , RXE = 1 , Receive only

    GPIO_PORTD_AMSEL_R  &= ~0xC0 ;
    GPIO_PORTD_DEN_R |= 0xC0 ;
    GPIO_PORTD_AFSEL_R |= 0xC0 ;
    GPIO_PORTD_PCTL_R = 0x11000000 ;
}


uint8_t ready() { //checking if the FIFO is not empty
    return ((UART2_FR_R & 0x10) == 0x10) ? 0 : 1;
}


unsigned char read() { //reads data from UART
    while(!ready()) {}
    return (unsigned char)(UART2_DR_R & 0xFF) ;
}



