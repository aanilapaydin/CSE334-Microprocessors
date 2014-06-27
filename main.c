/*****************************************************
*                                                    *
*  Ali Anil Apaydin                                  *
*  091044042                                         *
*                                                    *
*  CSE334_PROJECT_091044042                          *
*                                                    *
*  Description: That program is a simple autopark    *
*               simulation on HSC12.It calucates     *
*               entering time,speed of each car and  *
*               number of car that using park gate.  *
*                                                    *
*                                                    *
*  Date:01/06/2014                                   *
*                                                    *
*                                                    *
*                                                    *
******************************************************/

//Including libraries
#include <hidef.h>      
#include "derivative.h"      
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"


//Macros definitons for interrupts
#define disable() __asm(sei)
#define enable() __asm(cli)

//Var (10 cm * 1000 ms)
#define DISTANCE 10000
//Garage capacity
#define GARAGE_CAPACITY 10
//Gate speed maximum
#define SPEED_MAX 100

//Global variables definition
unsigned char selection;
volatile unsigned int first,
                      second,
                      time,
                      done,
                      autoNumber,
                      direction,
                      realTimeVar,
                      mainTime,
                      firstTime,
                      secondTime,
                      speed,
                      entered,
                      exited,
                      speedStat;
                      
unsigned int ms,sec,min,hour;                     

int stat1, stat2;

//Function declarations
void buzzer_speed_control(void);
unsigned char getFromKeypad(void); 
void calculateTimeSoFar(void);
void userReact(void);
void menu(void);
void userExit();

//Interrupts definition
interrupt(((0x10000-Vrti)/2)-1)void mainTimeInterrupt(void);
interrupt(((0x10000-Vtimch1)/2)-1)void sensor1Interrupt(void);
interrupt(((0x10000-Vtimch2)/2)-1)void sensor2Interrupt(void); 

//Main function
void main(void) {  
 
      //Begining
      char msg1[16] = "AutoPark";
      char msg2[16] = "Simulation";

      //Variables initialization
      autoNumber = 0;
      direction = FALSE;  
      realTimeVar = 0;
      mainTime = 0;
      speed=0;
      entered=0;
      exited=0;
      selection=0;
      speedStat=0;

      openlcd();         // setup LCD
      puts2lcd(msg1);    // write msg1
      put2lcd(0xC0,CMD); // move cursor to the second row
      puts2lcd(msg2);    // write msg2
      delay_1ms(100);
  
      //Ýnitialize interrupts
      disable();            // disable interrupts
      TSCR1 = 0X80;         // turn on timer subsystem
      TSCR2 = 0X06;         // set prescaler
      RTICTL = RTICTL | 0x37;  //set oscilator for 4.096 ms
      CRGINT = 0x80;           // real time 
      CRGFLG = 0x80;           // clear real time flag
      
      // Setup for input capture( PT1)
      TIOS = TIOS & ~0x02;                 // set PT1 for input capture
      TCTL4 = (TCTL4 | 0x04) & ~0x08;     // capture rising edge
      TFLG1 = 0X02;                       // clear flag
      TIE = TIE | 0x02;                   // enable channel1 interrupt

     // Setup for input capture( PT2 )
      TIOS = TIOS & ~0x04;                 // set PT1 for input capture
      TCTL4 = (TCTL4 | 0x10) & ~0x20;     // capture rising edge
      TFLG1 = 0X04;                       // clear flag
      TIE = TIE | 0x04;                   // enable channel2 interrupt

      enable();
  
      //Main while
      while(1){                            //enable interrupts
             
            menu();                             //menu function call            
            
            selection = getFromKeypad();         //get from keypad it is not a loop
            
            
            switch(selection){                   //switch for user selection
              case '1':
              case '2':
              case '3':
                userReact();                     //user 1 2 3 for seleciton
              break;
              case '4':
                userExit();                      //4 for exit 
              default:
              break;           
            }
  
            //Update for sensor  variables if
            if( stat1 == 1 && stat2 == 1 ) {
              
               //if car entered
               if( direction == TRUE ) {
                    //garage capacity control
                    if(autoNumber==GARAGE_CAPACITY){
                      sprintf(msg1,"%s","FULL");
                      put2lcd(0x01,CMD);  // clear display , move cursor home
                      delay_1ms(2);       // wait until clear display command complete
                      puts2lcd(msg1); 
                      delay_1ms(50);
                    }
                    //enter car
                    if(autoNumber<GARAGE_CAPACITY){ 
                      autoNumber = autoNumber + 1;
                      time = secondTime - firstTime;
                      speed = DISTANCE/time;
                      speedStat=1;
                      entered = entered + 1;
                    } 
   
                 }
                 //if car exit
                 else if ( direction == FALSE ) {
                    //if no car at garage  
                    if(autoNumber==0){} 
                    //exit car
                    else{   
                      autoNumber = autoNumber - 1;
                      time = firstTime - secondTime;
                      speed = DISTANCE/time;
                      speedStat=1;
                      exited = exited + 1;
                    }
                      
               
                 }
                 //stats update
                 stat1 = 0;
                 stat2 = 0;
            }
            //speed prinbt and play buzzer if speed is high
            if(done>0 && done%2==0 && speedStat==1){
              buzzer_speed_control();
            }


      }// end of while(1)
   
   __asm(swi);
   
 }// end of main
 
//user exit function 
void userExit(){ 
  //Screen printing and exit
  char msg1[16] = "GoodBye!";
  openlcd();
  puts2lcd(msg1);
  delay_1ms(50);
  __asm(swi);
}
//calculate according to real time 
void calculateTimeSoFar(){   
  ms=mainTime%1000;
  sec=mainTime-ms;
  sec=sec/1000;
  ms=sec%60;
  min=sec-ms;
  min=min/60;
  ms=min%60;
  hour=min-ms;
  hour=hour/60;
}
// user selection function
void userReact(){
 
  char msg1[16];
  char msg2[16];
  unsigned int ctr;
 
 //if user pressed 1
 //print time so far
  if(selection=='1'){
     for(ctr=0;ctr<=4;){
       sprintf(msg1,"%s","Time so Far:");
       sprintf(msg2,"%u:%u:%u",hour,min,sec);
       openlcd();        
       puts2lcd(msg1);    // write msg1
       put2lcd(0xC0,CMD); // move cursor to the second row
       puts2lcd(msg2);    // write msg2
       delay_1ms(1);
       ctr = ctr + 2;
     }
  }
  // if user pressed 2
  // print entered and exited car so far
  if(selection=='2'){
     for(ctr=0;ctr<=4;){
       sprintf(msg1,"Entered Cars:%u",entered);
       sprintf(msg2,"Exited Cars:%u",exited);        
       openlcd();
       puts2lcd(msg1);    // write msg1
       put2lcd(0xC0,CMD); // move cursor to the second row
       puts2lcd(msg2);    // write msg2
       delay_1ms(1);
       ctr = ctr + 2;
     }
  }
  // if user pressed 3
  // print average car for a minute
  if(selection=='3'){
     for(ctr=0;ctr<=4;){
       sprintf(msg1,"%s","Average Cars");
       // divide by zero control
       if(min==0){
        sprintf(msg2,"For a Minute:%u",(entered+exited));        
       } else{
        sprintf(msg2,"For a Minute:%u",(entered+exited)/((hour*60)+min));        
       }
       openlcd();
       puts2lcd(msg1);    // write msg1
       put2lcd(0xC0,CMD); // move cursor to the second row
       puts2lcd(msg2);    // write msg2
       delay_1ms(1);
       ctr = ctr + 2;
     }
  }
 

}
// menu function 
void menu(){
  
   char msg1[16] = "MENU";
   char msg2[16] = "1)Total Time";
   openlcd();
   puts2lcd(msg1);    // write msg1
   put2lcd(0xC0,CMD); // move cursor to the second row
   puts2lcd(msg2);    // write msg2
   delay_1ms(1);
   sprintf(msg1,"%s","1)Total Time");
   sprintf(msg2,"%s","2)Total Cars");
   openlcd();
   puts2lcd(msg1);    // write msg1
   put2lcd(0xC0,CMD); // move cursor to the second row
   puts2lcd(msg2);    // write msg2
   delay_1ms(1);
   sprintf(msg1,"%s","2)Total Cars");
   sprintf(msg2,"%s","3)Average Cars");
   openlcd();
   puts2lcd(msg1);    // write msg1
   put2lcd(0xC0,CMD); // move cursor to the second row
   puts2lcd(msg2);    // write msg2
   delay_1ms(1);
   sprintf(msg1,"%s","3)Average Cars");
   sprintf(msg2,"%s","per minute");
   openlcd();
   puts2lcd(msg1);    // write msg1
   put2lcd(0xC0,CMD); // move cursor to the second row
   puts2lcd(msg2);    // write msg2
   delay_1ms(1);
   sprintf(msg1,"%s","per minute");
   sprintf(msg2,"%s","4)Exit");
   openlcd();
   puts2lcd(msg1);    // write msg1
   put2lcd(0xC0,CMD); // move cursor to the second row
   puts2lcd(msg2);    // write msg2
   delay_1ms(1);
   sprintf(msg1,"%s","4)Exit");  
   openlcd();
   puts2lcd(msg1);    // write msg1 
   delay_1ms(1);
}
// speed print and control and play buzzer control
// function
void buzzer_speed_control(){

  char msg1[16]="";
  unsigned int ctr; 
                                                                                                                                                                                                     
  sprintf(msg1,"Speed:%u",speed);
  openlcd();
  puts2lcd(msg1); 
  delay_1ms(2);    
  if(speed>SPEED_MAX){   
    for (ctr=0;ctr<=16;) 			
    {
      DDRT = DDRT | 0x20;
      PTT = PTT | 0x20;      
      delay_1ms(5);    
      PTT = PTT & 0xDF;      
      delay_1ms(5);
      ctr=ctr+2;
    }   
  }
  
  speedStat=0;
}
  
//get from keypad function
// it is not a loop
// it check only one iteration for user press
unsigned char getFromKeypad(void) 
{
  const unsigned char keypad[4][4] = {'1','2','3','A',  
                                      '4','5','6','B',  
                                      '7','8','9','C',  
                                      '*','0','#','D'};
  unsigned char row;
  unsigned int col;
  unsigned char result = '\0';
   
  DDRA = 0xF0;
   
  // column loop
  for(col = 0 ; col < 4 ; ++col) {
      
      PORTA = 0xEF << col;
      row = PORTA & 0x0F;
      
      switch(row) {
      
          case 0x0E:
              return keypad[0][col];
          case 0x0D:
              return keypad[1][col];
          case 0x0B:
              return keypad[2][col];
          case 0x07:
              return keypad[3][col];
      }
  }
  
  return result;
}
// real time interrupt
interrupt(((0x10000-Vrti)/2)-1)void mainTimeInterrupt(void){

  realTimeVar++;
  mainTime=realTimeVar*4;  // calculation maintime (miliseconds)
  calculateTimeSoFar();    // calculation hour,min,sec accotding to maintime
  CRGFLG = 0x80;

}
// sensor 1 interrupt
interrupt(((0x10000-Vtimch1)/2)-1)void sensor1Interrupt(void){

  //update sensor 1 variables
  //and input capture
  stat1=1;
  direction=FALSE;
  done++;
  first = TC1;
  firstTime = mainTime;
  TFLG1 = 0x02;
}
// sensor 2 interrupt
interrupt(((0x10000-Vtimch2)/2)-1)void sensor2Interrupt(void){
  
  // update sensor 2 variables
  // and input capture 
  stat2=1;
  direction=TRUE;
  done++;
  second = TC2;
  secondTime = mainTime;
  TFLG1 = 0x04;
}
