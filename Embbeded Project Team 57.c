#include "tm4c123gh6pm.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"

void SystemInit(){}
	
void milli_delay(int n) //software delay
	{
    int i , j ;
for( i=0;i<n;i++) {
for(j=0;j<3180;j++){}
}
}


void lcd_cmd(unsigned char cmd)
	{

GPIO_PORTA_DATA_R &= ~(0xE0) ; //RS = RW = E = 0 
GPIO_PORTB_DATA_R = cmd ; //write command
GPIO_PORTA_DATA_R |= 0x40 ; // Enable on
milli_delay(10) ;
GPIO_PORTA_DATA_R &= ~(0xE0) ; // Enable off 
milli_delay(10) ;


}

void lcd_data(unsigned char data) {
GPIO_PORTA_DATA_R |= 0x80  ; //RS = 1
GPIO_PORTA_DATA_R &= ~(0x60); // E = RW = 0 
GPIO_PORTB_DATA_R = data; //write data
GPIO_PORTA_DATA_R |= 0x40; // Enable on
milli_delay(10) ;
GPIO_PORTA_DATA_R &= ~(0x60) ; // Enable off
milli_delay(10) ;
}


void lcd_init() {
	
SYSCTL_RCGCGPIO_R |= 0x03 ; // Initialize all clocks for ports A,B 
while( (SYSCTL_PRGPIO_R & 0x03) == 0 ) {}

    
//Digital Enable for LCD pins
    GPIO_PORTA_DEN_R |= 0xE0 ; 
    GPIO_PORTB_DEN_R |= 0xFF ;
//All pins are output pins
    GPIO_PORTA_DIR_R |= 0xE0 ;
    GPIO_PORTB_DIR_R |= 0xFF ;
//No analog mode
    GPIO_PORTA_AMSEL_R &= ~(0xE0) ;
    GPIO_PORTB_AMSEL_R &= ~(0xFF) ;
//No alternate functions
    GPIO_PORTA_AFSEL_R &= ~(0xE0) ;
    GPIO_PORTB_AFSEL_R &= ~(0xFF) ;

    GPIO_PORTA_PCTL_R &= 0x000FFFFF ;
    GPIO_PORTB_PCTL_R  = 0 ;
	
	//some lcd commands
	lcd_cmd(0x30);  //wake up 
  milli_delay(10);
  lcd_cmd(0x38);  //8-bit bus,2 line, 5x8 dots display mode
  milli_delay(10);
  lcd_cmd(0x01);  //clear
  milli_delay(10);
  lcd_cmd(0x0F);  //diplay is on
  milli_delay(10);

}
//Note : first line and second line lcd commands will be used in the main function