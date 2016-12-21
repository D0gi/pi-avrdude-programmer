#include <util/delay.h>
#include <avr/io.h>          
int main (void) {            
  DDRA  = 0xff;             
  while(1) {                
    PORTA = 0xff;
    _delay_ms(500);
    PORTA = 0x00;
    _delay_ms(500);
  }                         
 return 0;                 
}
