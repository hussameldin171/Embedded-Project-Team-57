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

/*the following functions are responsible for converting the latitude and longitude format into double values and
 changing the degrees and decimal minutes to only degrees
 */


double get_lat(char lat[]) {
    char *ptr;
    double latitude = strtod(lat , &ptr) ;
    double part1 = (int)latitude/100 ;
    double part2 = (latitude/100 - part1)*100 ;
    part2/=60 ;
    return part1+part2 ;
}



double get_lon(char lon[]) {
    char *ptr;
    double longitude = strtod(lon , &ptr) ;
    double part1 = (int)(longitude/100) ;
    double part2 = (longitude/100- part1)*100 ;
    part2/=60 ;
    return part1+part2 ;
}






/*****LED and Button******/


    void Init_Port_F (){

    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20 ) == 0 ){}

        // PORT F initializing led at PF3 and switch at PF0
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R = 0x09;
    GPIO_PORTF_AFSEL_R &= ~(0x09);
    GPIO_PORTF_AMSEL_R &= ~(0x09);
    GPIO_PORTF_PCTL_R =0;
        GPIO_PORTF_DEN_R |= 0x09;
    GPIO_PORTF_DIR_R |= 0x08; // led at PF3 is output
    GPIO_PORTF_DIR_R &= 0xFE; // Switch at PF0 is input
    GPIO_PORTF_PUR_R |= 0x01; //pull up resistance for push button

    }


void led_on(double distance) //led is turned on when the distance exceeds 100m
    {
    if (distance >= 100)
        GPIO_PORTF_DATA_R |= 0x08 ;
}










