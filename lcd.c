#include "derivative.h"      /* derivative-specific definitions */
#include "lcd.h" 
  
void openlcd(void){
  DDRK = 0xFF;        // make portk output
  delay_1ms(30);     // wait for LCD to get ready
  put2lcd(0x28,CMD);  // 4 bit data, 2 line display, 5x7 font
  put2lcd(0x0f,CMD);  // turn on LCD,cursor,blinking
  put2lcd(0x06,CMD);  // move cursor right after writing a character
  put2lcd(0x01,CMD);  // clear display , move cursor home
  delay_1ms(2);       // wait until clear display command complete
}

void puts2lcd(char *ptr){
  while(*ptr){
    put2lcd(*ptr,DATA);
    delay_50us(1);
    ++ptr;
  }
}


void put2lcd(char c, char type){
  char c_hi,c_lo;
  
  c_hi = (c & 0xF0) >> 2;
  c_lo = (c & 0x0F) << 2;
  
  if( type == DATA )
    PORTK =  PORTK | 0x01;   // if data, make PK0 1
  else
    PORTK =  PORTK & (~0x01);  // if command, make PK0 0
 
  if( type == DATA )
    PORTK =  c_hi | 0x02 | 0x01;   
  else
    PORTK =  c_hi | 0x02;  
 
  __asm(nop);               // wait for write process 
  __asm(nop);
  __asm(nop);
  
  PORTK = c_hi  & (~0x02);    // make PK1(E) low
  
  delay_50us(1);
  
  if( type == DATA )
    PORTK = ( PORTK) | 0x01;   // if data, make PK0 1
  else
    PORTK = ( PORTK) & (~0x01);  // if command, make PK0 0
 
  if( type == DATA )
    PORTK = ( c_lo) | 0x02 | 0x01;   
  else
    PORTK = ( c_lo) | 0x02;  
  
  __asm(nop);               // wait for write process 
  __asm(nop);
  __asm(nop);
  
  PORTK =  c_lo & (~0x02);    // make PK1(E) low
  
  delay_50us(1);
}



                                       
void delay_50us(int n){
   volatile int c;
   
   for(;n>0;n--)
    for(c=133;c>0;c--);
}


void delay_1ms(int n){
  for(;n>0;n--)
    delay_50us(200);
}
