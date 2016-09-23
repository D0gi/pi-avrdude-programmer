// CESS. Lesson 4
// Prof. J. Wagner
// idea of i2c communication stolen from; 
//   http://www.atmel.com/Images/atmel-2565-using-the-twi-module-as-i2c-slave_applicationnote_avr311.pdf
//   http://www.nongnu.org/avr-libc/user-manual/group__util__twi.html
//   https://github.com/kelvinlawson/avr311-twi-slave-gcc/blob/master/standard/TWI_slave.c
//   http://www.mikrocontroller.net/topic/402753


// General to the language C
#include <stdint.h>  // bittrue integers
#include <stdbool.h> // use latest boolean variables with old C compilers

// Special to the microcontoller family
#include <avr/io.h>  // register names
#include <avr/interrupt.h> // names of interrupts

// Special to hardware programming
#include <util/twi.h> // status of I2C interface

//delay functions, not for real world use, Energy!
#define F_CPU 8000000
#include <util/delay.h> 


// Graphic memory
#define MAX_GRAPHIC 12
//----------------------------------------HHLLLLLLLL
//----------------------------------------||||||||||
volatile uint16_t pixel[MAX_GRAPHIC] ={ 0b1000000000, //0x00
                                        0b0100000000, //0x02
                                        0b0010000000, //0x04
                                        0b0001000000, //0x06
                                        0b0000100000, //0x08
                                        0b0000010000, //0x0A 
                                        0b0000010000, //0x0C
                                        0b0000100000, //0x0E
                                        0b0001000000, //0x10
                                        0b0010000000, //0x12
                                        0b0100000000, //0x14
                                        0b1000000000  //0x16
//----------------------------------------||||||||||
};



#define FLANK PORTB |= (1 << PB3); PORTB &= ~(1 << PB3);
#define STR_1 PORTB |= (1 << PB2); 
#define STR_0 PORTB &= ~(1 << PB2);
#define SET_1 PORTB |= (1 << PB4); 
#define SET_0 PORTB &= ~(1 << PB4); 

#define TWI_SLAVE_ADDRESS 0x03

// Display functions
void WriteLine(uint16_t line);
void WriteRow(uint8_t row);


int main(void)
{
  // Test the memory
  /*for (int i = 0; i< 2*MAX_GRAPHIC; i++)
     ((uint8_t*)pixel) [i] = i;*/

  // init slave
  //TWAMR=0xff; sollte eigentlich die adressbits maskieren, existiert aber wohl nicht
  TWAR = TWI_SLAVE_ADDRESS << 1;
  TWCR  = _BV(TWEN) // switch on I2C (TWI) interface
        | _BV(TWEA) // automatic acknowledge (ACK) if slave adress is read
        | _BV(TWIE);// allow TWI interrupt

  sei(); //allow interrupt globaly



  // setup Bits for Display
  DDRC = 0x0f; 
  DDRB = 0xff; 
  DDRD = 0xf0; 


// Setze alle Ausgaenge auf 0
  PORTB = PORTC = PORTD = 0x00; 

  for (int i = 0; i < 16; i++) {
        WriteRow(1);
  }

 

  while(1)
  {
    for (uint8_t row = 0; row < 12; row++)
	  {   
      WriteRow(row);
      WriteLine(pixel[row]);
      _delay_ms(1);
    } // for row
  } // while 1

} //end main



//Clear busy flag, enable TWI, enable TWI-INT
#define TWCR_init (_BV(TWINT) | _BV(TWEN) | _BV(TWIE))

volatile uint8_t TWI_internal_address=0;    // address inside circuit
volatile bool TWI_internal_address_valid = false; // the address is valid



// interrupt service for TWI
// this thing works, but for differnet tasks, you have to change it
// programming this was a mess, handle with care
ISR(TWI_vect)
{
  /*TWAR += 1;
  if(TWAR > (0x77 << 1)){TWAR = (0x03 << 1);}*/

  // mask prescaler bits from TWI status register
  // ref to:
  //    http://www.nongnu.org/avr-libc/user-manual/group__util__twi.html

  switch (TWSR & TW_STATUS_MASK )
  {

    // Slave receive_command, slave notices its (external) address, slave retruns ACK 
    case TW_SR_SLA_ACK:
   
      //auto ACK next byte 

      TWCR = TWCR_init | (1 << TWEA);

	  // internal address not valid
      TWI_internal_address_valid = false;
    break;
  
    // slave receive_command, slave gets data byte
    case TW_SR_DATA_ACK:
      
	  // if it is the first, it is the internal address
      if(!TWI_internal_address_valid)
      {
         TWI_internal_address = TWDR;
	     TWI_internal_address_valid = true;
        
		 // if the internal address stays within graphic buffer 
	     if (TWI_internal_address < 2*MAX_GRAPHIC)
           // auto ASK next byte 
		   TWCR = TWCR_init | _BV(TWEA);
         else
		   // dismiss
	       TWCR = TWCR_init;
       }
	   // internal address is valid, it is the second or later data byte
       else 
       {
          // write data byte into graphic RAM
          ((uint8_t*)pixel)[TWI_internal_address]= TWDR;
    
	      // increment for next data byte 
	      TWI_internal_address++;

          // if the internal address stays within graphic buffer 
	     if (TWI_internal_address < 2*MAX_GRAPHIC)
           // auto ASK next byte 
		   TWCR = TWCR_init | _BV(TWEA);
         else
		   // dismiss
	       TWCR = TWCR_init;
       }
    
	break;
	 
    // slave receive_command, gets data byte, but out of range and therefore not ACK 
    case TW_SR_DATA_NACK:
      // master will end transmission, slave should auto ASK if master sends address again
      TWCR = TWCR_init | _BV(TWEA);
    break;

    // slave is waiting for data bytes, but master ends transmission 
    case TW_SR_STOP:
      // master will ends transmission, slave should auto ASK if master sends address again
      TWCR = TWCR_init | _BV(TWEA);
    break;



    // slave transmit_command, slave ACK
    case TW_ST_SLA_ACK:

  
      if(!TWI_internal_address_valid)
  	  {
        TWI_internal_address_valid=true;
	    TWI_internal_address = 0;
      }

      // $$$ Bus is too slow -> sometimes Bit 7 is always set, sorry
	    _delay_us(10);
	    TWDR = ((uint8_t*)pixel)[TWI_internal_address];
	    TWI_internal_address++;

      // if there is RAM left, master may read more
      if(TWI_internal_address < 2*MAX_GRAPHIC) 
        TWCR = TWCR_init | _BV(TWEA); 
      else
        // Do not ACK next read, send only 1-Bits
        TWCR = TWCR_init;     
	  break;

  	// slave transmit_command, slave ACK
    case TW_ST_DATA_ACK:
    
        if(!TWI_internal_address_valid)
    	  {
          TWI_internal_address_valid=true;
  	    TWI_internal_address = 0;
        }

  	  TWDR = ((uint8_t*)pixel)[TWI_internal_address];
      
  	  TWI_internal_address++;

        // if there is RAM left, master may read more
        if(TWI_internal_address < 2*MAX_GRAPHIC) 
          TWCR = TWCR_init | _BV(TWEA); 
        else
          // Do not ACK next read, send only 1-Bits
          TWCR = TWCR_init;
  	break;

    // slave transmitted data, but master did not ACK, wants not more data
    case TW_ST_DATA_NACK:
    
	  // auto ACK if slave hears it name next time
      TWCR = TWCR_init | (1 << TWEA);
    break;

    // Master receives last date byte (of a series)
    case TW_ST_LAST_DATA:
      // auto ACK if slave hears it name next time
      TWCR = TWCR_init | (1 << TWEA);
    break;

    // Something nasty happen, auto ACK if you hear your name next time, recover from error 
    //  (in slave mode no STOP will be send, but it resets internal FSM, TWSTO auto resets after)
    default:

      TWCR = TWCR_init | _BV(TWEA) | _BV(TWSTO);
    break; 
  } // end switch

}

void WriteLine(uint16_t muster)
{
  PORTB = PORTC = PORTD = 0x00; 
  PORTB |= (((muster >> 9) & 1) << PB1); // Zeile1 
  PORTB |= (((muster >> 8) & 1) << PB0); // Zeile2 
  PORTD |= (((muster >> 7) & 1) << PD7); // Zeile3 
  PORTD |= (((muster >> 6) & 1) << PD6); // Zeile4 
  PORTD |= (((muster >> 5) & 1) << PD5); // Zeile5 
  PORTD |= (((muster >> 4) & 1) << PD4); // Zeile6 
  PORTC |= (((muster >> 3) & 1) << PC3); // Zeile7 
  PORTC |= (((muster >> 2) & 1) << PC2); // Zeile8 
  PORTC |= (((muster >> 1) & 1) << PC1); // Zeile9 
  PORTC |= (((muster >> 0) & 1) << PC0); // Zeile10 
}


void WriteRow(uint8_t row)
{
  if(row == 0){SET_0}else{SET_1}
  STR_1 
  FLANK
  STR_0
}
