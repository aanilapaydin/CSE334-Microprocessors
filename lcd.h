#define DATA 1
#define CMD 0 

void openlcd(void); // initialize LCD display
void put2lcd(char c,char type); // write data/command to LCD display
void puts2lcd(char* ptr); // write string to LCD display
void delay_50us(int n); // delay n*50 microsecond
void delay_1ms(int n); // delay n*1 milisecond
