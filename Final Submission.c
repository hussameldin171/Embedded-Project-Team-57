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




/****** LCD ********/


//PA7 -- > RS
//PA5 -- > RW
//PA6 -- > E
//PBn --> Dn
void lcd_cmd(unsigned char cmd) { //send a command to lcd

GPIO_PORTA_DATA_R &= ~(0xE0) ; // disable RS , Enable , RW
GPIO_PORTB_DATA_R = cmd ;
GPIO_PORTA_DATA_R |= 0x40 ; // Enable on
milli_delay(10) ;
GPIO_PORTA_DATA_R &= ~(0xE0) ; // Enable off
milli_delay(10) ;


}

void lcd_data(unsigned char data) { //write a character on lcd
GPIO_PORTA_DATA_R |= 0x80  ; //Enable RS
GPIO_PORTA_DATA_R &= ~(0x60); // disable Enable , RW
GPIO_PORTB_DATA_R = data;
GPIO_PORTA_DATA_R |= 0x40; // Enable on
milli_delay(10) ;
GPIO_PORTA_DATA_R &= ~(0x60) ; // Enable off
milli_delay(10) ;
}


void lcd_init() {
SYSCTL_RCGCGPIO_R |= 0x2B ; // Initialize all clocks for ports A,B,D, and F
while( (SYSCTL_PRGPIO_R & 0x2B) == 0 ) {}

    //GPIO_PORTB_LOCK_R = 0x4C4F434B;
    //GPIO_PORTB_CR_R = 0xFF;

    GPIO_PORTA_DEN_R |= 0xE0 ;
    GPIO_PORTB_DEN_R |= 0xFF ;

    GPIO_PORTA_DIR_R |= 0xE0 ;
    GPIO_PORTB_DIR_R |= 0xFF ;

    GPIO_PORTA_AMSEL_R &= ~(0xE0) ;
    GPIO_PORTB_AMSEL_R &= ~(0xFF) ;

    GPIO_PORTA_AFSEL_R &= ~(0xE0) ;
    GPIO_PORTB_AFSEL_R &= ~(0xFF) ;

    GPIO_PORTA_PCTL_R &= 0x000FFFFF ;
    GPIO_PORTB_PCTL_R  = 0 ;

}



void display(char word[] ){

    int k   = strlen(word);
    int i = 0;
    for (i =0 ; i <k; i++)
    {
        lcd_data(word[i]);
        milli_delay(100);
    }
}

void distance_display(int x){ //displays the distance on lcd as integer

    char init[10] , final[10];
    int i = 0 ;
    while(x>0) {
        init[i++] = x%10 +'0' ;
        x/=10 ;
        if(i == 10){break;}
    }
    int length = strlen(init);
        for(i = 0 ; i < strlen(init) ; i++){
            final[length-i-1] = init[i] ;
        }

    display(final) ;
    }






void getData() //Parses data and stores the latitude and longitude as strings in global variables
{

    int check = 0; //to check if the gppga tag is reached, otherwise starts again
    int i ; //for a loop
    char lat[12] , lon[13] ; //to store latitude and longitude strings
  unsigned char temp = 0 ; //to read characters from UART
  int index = 0 ;
  start:
    temp = read() ;
    if (temp == '$') {
            temp = read() ;
            if (temp == 'G') {
                    temp = read() ;
                    if (temp == 'P') {
                            temp = read() ;
                            if (temp == 'G') {
                                    temp = read() ;
                                    if (temp == 'G') {
                                            temp = read() ;
                                          if (temp == 'A') {
                                                    check = 1 ; //GPPGA tag reached

                                                    temp = read();

                                                    temp = ' ';
                                                    while(temp != ',') //skip until next comma
                                                        {

                                                        temp = read() ;

                                                    }
                                                        //we reached the other comma, now we'll parse the latitude string
                                                    temp = read();
                                                    while(temp != ',') {
                                                        lat[index] = temp ;

                                                        temp = read();
                                                        index++;


                                                    }




                                                    index = 0 ;
                                                    temp =  ' ' ;
                                                    while(temp != ',') {temp = read();} //skip North or South

                                                    temp = read();
                                                    //Now parsing longitude string
                                                    while(temp != ',') {
                                                        lon[index] = temp ;

                                                        temp = read();
                                                        index++;
                                                    }

                                                    //storing parsed data in global variables
                                                    strcpy(latitude , lat);
                                                    strcpy(longitude, lon);

                                                }


                                    }
                                        }

    }
}



}
if (check == 0) {goto start;} //if it'snot the gppga tag, it restarts its operation until it reaches the desired tag
    }





/*******Distance Calculation******/




//haversine rule for calculating the total distance between two points given their latitudes and longitudes

double hav(double lat1, double lat2, double lon1, double lon2){  //lat1&2 and lon1&2 are in radian
    double d,h,sin_latdiff,sin_londiff,cos_lat1,cos_lat2,sin_latdiff_sq,sin_londiff_sq ;
    //degree to rad conversion first
    lat1*= pi/180 ; lat2*= pi/180 ; lon1*= pi/180 ; lon2*= pi/180 ;

    sin_latdiff             = sin((lat2-lat1)/2);
    sin_londiff             = sin((lon2-lon1)/2);
    cos_lat1                    = cos(lat1);
    cos_lat2                    = cos(lat2);
    sin_latdiff_sq      = sin_latdiff*sin_latdiff;
    sin_londiff_sq      = sin_londiff*sin_londiff;

    h  = sin_latdiff_sq + cos_lat1*cos_lat2*sin_londiff_sq;

    d = 2*r*asin(sqrt(h));
    return(d);
}

//A function that returns the total distance if it exceeds 100m or when the push button at PF0 is pressed
double total_distance(){
    double distance = 0.0;
    getData() ; //Data is parsed and latitude and longitudes parsed strings and stored in global variables

    double old_lat = get_lat(latitude) ;
    double old_lon = get_lon(longitude);


    double new_lat , new_lon ;

   while ( (distance <= 100.0) ) //loop with condition that distance exceeds 100
   {
       lcd_cmd(0X01);
      getData() ;
      new_lat = get_lat(latitude) ;
        new_lon = get_lon(longitude);

        distance += hav(old_lat,new_lat,old_lon, new_lon);
        old_lat = new_lat;
        old_lon = new_lon ;
        distance_display(distance); //print distance constantly
        milli_delay(2000) ;

    }
    led_on(distance) ; //turn on led after exceeding 100m

    return distance ;

}



/********************************************************************/
int main(void)
{
    double distance ;
    //Initializing all required ports
    Init_Port_F () ;
    lcd_init();
    UART2_Init();



               lcd_cmd(0x30);

               milli_delay(10);
               lcd_cmd(0x38);  //8-bit mode, 2 line display mode, 5x8 dots display mode
               milli_delay(40);
               lcd_cmd(0x01);  //clear display
               milli_delay(40);
               lcd_cmd(0x0F);  //display On
               milli_delay(40);
               lcd_cmd(0x80);  //First line


               milli_delay(7000) ;//Delay until the gps works correctly
               distance = total_distance(); //prints and calculates the distance constantly and turns led on
               lcd_cmd(0xc0);//Second Line
               display("Target reached");
}














