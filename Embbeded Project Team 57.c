#include "tm4c123gh6pm.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#define r 6371*1000

void SystemInit(){}
	

void UART_INT() {         
	// Initializing UART2 in PORT D 
	//PD6 --> Rx
	//PD7 --> Tx
	
	SYSCTL_RCGCUART_R |= 0x08 ;
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


uint8_t ready() {
	return ((UART2_FR_R & 0x10) == 0x10) ? 0 : 1;
}

unsigned char read() {
	while(!ready()) {}
	return (unsigned char)(UART2_DR_R & 0xFF) ;
}




















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


void lcd_init() {    // lcd intialiazation function
	
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

void display(char word[] ) //a function that takes a string paramter and displays it on the lcd
	{

    int k   = strlen(word);
    int i = 0;
    for (i =0 ; i <k; i++)
    {
        lcd_data(word[i]);
        milli_delay(100);
    }
}

void show_distance(double distance) //a function that displays the total distance on lcd
	{
		lcd_cmd(0x80);
	  char d[10]; //maximum size of the number
    sprintf(d, "%g", distance);
		display("Distance = ") ;
		lcd_cmd(0XC0);
		display(d) ;
}








//haversine rule for calculating the total distance between two points given their latitudes and longitudes

double hav(double lat1, double lat2, double lon1, double lon2){  //lat1&2 and lon1&2 are in radian
	double d,h,sin_latdiff,sin_londiff,cos_lat1,cos_lat2,sin_latdiff_sq,sin_londiff_sq ;
	
	sin_latdiff 			= sin((lat2-lat1)/2);
	sin_londiff 			= sin((lon2-lon1)/2);
	cos_lat1 					= cos(lat1);
	cos_lat2					= cos(lat2);
	sin_latdiff_sq 		= sin_latdiff*sin_latdiff;
	sin_londiff_sq		= sin_londiff*sin_londiff;
	
	h  = sin_latdiff_sq + cos_lat1*cos_lat2*sin_londiff_sq;
	
	d = 2*r*asin(sqrt(h));
	return(d);
};






//Two functions that return the latitude and longitude for a certain reading and shall be defined later 
double get_lat(); 
double get_long();


//A function that returns the total distance if it exceeds 100m or when the push button at PF0 is pressed
double total_distance(){
	double distance = 0.0;
	double old_lat = get_lat();
	double old_long = get_long();
	
	double new_lat , new_long ;
	
	while ( (distance <= 100.0) && ( (GPIO_PORTF_DATA_R & 0x01) == 0x01) )
	{
	  new_lat =  get_lat();
		new_long = get_long();
		distance += hav(old_lat,new_lat,old_long, new_long);
		old_lat = new_lat;
		old_long = new_long ;
	}
	return distance ;

}








