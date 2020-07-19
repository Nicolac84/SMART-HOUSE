#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <string.h>

#define BAUD 19600
#define MYUBRR (F_CPU/16/BAUD-1)



// configuration bits for PWM
// fast PWM, 8 bit, non inverted
#define TCCRA_MASK (1<<WGM10)|(1<<COM1C0)|(1<<COM1C1)|(1<<COM1B1)|(1<<COM1B0) |(1<<COM1A1) |(1<<COM1A0)
#define TCCRB_MASK ((1<<WGM12)|(1<<CS10))
// attivazione ambaradam 2 ,3 e5
#define TCCRA3_MASK (1<<WGM30)|(1<<COM3C0)|(1<<COM3C1)|(1<<COM3B1)|(1<<COM3B0) |(1<<COM3A1) |(1<<COM3A0)
#define TCCRB3_MASK ((1<<WGM32)|(1<<CS30))

// attivazione ambaradam 6, 7 e 8 
#define TCCRA4_MASK (1<<WGM40)|(1<<COM4C0)|(1<<COM4C1)|(1<<COM4B1)|(1<<COM4B0) |(1<<COM4A1) |(1<<COM4A0)
#define TCCRB4_MASK ((1<<WGM42)|(1<<CS40))


#define BAUD 19600
#define MYUBRR (F_CPU/16/BAUD-1)

void UART_init(void){
  // Set baud rate
  UBRR0H = (uint8_t)(MYUBRR>>8);
  UBRR0L = (uint8_t)MYUBRR;

  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* 8-bit data */ 
  UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);   /* Enable RX and TX */  

}

void UART_putChar(uint8_t c){
  // wait for transmission completed, looping on status bit
  while ( !(UCSR0A & (1<<UDRE0)) );

  // Start transmission
  UDR0 = c;
}

uint8_t UART_getChar(void){
  // Wait for incoming data, looping on status bit
  while ( !(UCSR0A & (1<<RXC0)) );
  
  // Return the data
  return UDR0;
    
}

// reads a string until the first newline or 0
// returns the size read
uint8_t UART_getString(uint8_t* buf){
  uint8_t* b0=buf; //beginning of buffer
  while(1){
    uint8_t c=UART_getChar();
    *buf=c;
    ++buf;
    // reading a 0 terminates the string
    if (c==0)
      return buf-b0;
    // reading a \n  or a \r return results
    // in forcedly terminating the string
    if(c=='\n'||c=='\r'){
      *buf=0;
      ++buf;
      return buf-b0;
    }
  }
}

void UART_putString(uint8_t* buf){
  while(*buf){
    UART_putChar(*buf);
    ++buf;
  }
}





#define MAX_BUF 256
int main(void){

// we will use timer 1
  TCCR1A=TCCRA_MASK;
  TCCR1B=TCCRB_MASK;
  // contatori pin 2 ,3 e 5
  TCCR3A=TCCRA3_MASK;
  TCCR3B=TCCRB3_MASK;
 
    // contatori pin 6, 7 e 8
  TCCR4A=TCCRA4_MASK;
  TCCR4B=TCCRB4_MASK;

  // clear all higher bits of output compare for timer
  OCR1AL=155;
  OCR1BL=10;
  OCR1CH=155;
  OCR1CL=155;

// regolatori volt  pin 2,3 e 5
  
  OCR3AL =100;   //pin 5
  OCR3BL=200;    //pin 2
  OCR3CL=55;     //pin 3




// regolatori volt  pin 6, 7 e 8 
  
  OCR4AL=150;   //pin 6
  OCR4BL=200;   //pin 7 
  OCR4CL=55;    //pin 8




  // the LED is connected to pin 13
  // that is the bit 7 of port b, we set it as output
  const uint8_t mask=(1<<5)|(1<<6)|(1<<4)|(1<<3);
  // we configure the pin as output
  DDRB = mask; //mask;
// maschera attivazione pin 2 e 3
  DDRE =mask;

// maschera attivazione pin 6,7 e 8
  DDRH =mask;



 // uint8_t intensity=0;
//  while(1){
    // we write on the output compare register a value
    // that will be proportional to the opposite of the
    // duty_cycle
 
   //OCR1AL =250 ;
    
   // printf("v %u\n", (int) OCR1AL);
   // _delay_ms(100); // from delay.h, wait 1 sec
    
   // intensity+=0;
  
//}


  UART_init();
  UART_putString((uint8_t*)"READY\n");
  uint8_t buf[MAX_BUF];

  
 while(1) {
 
    UART_getString(buf);

    UART_putString((uint8_t*)"received\n");
    UART_putString(buf);
   if(strcmp("pinon\n",buf)==0){       
        UART_putString((uint8_t*)"Scegli un pin da 2-8 \n"); 
        UART_getString(buf);
        int pin=atoi(buf);
        if((pin <8) || (pin >1)){
    
         UART_putString((uint8_t*)"inserisci valore 0-255 per dimmerare per il pin \n");

         UART_getString(buf);
         int f=atoi(buf);
      
         switch (pin){
         case 2:  OCR3BL=f; UART_putString((uint8_t*)"valore settato per pin 2 "); break;   
         case 3:  OCR3CL=f; UART_putString((uint8_t*)"valore settato per pin 3 "); 
         default:     
              OCR4BL=255;
         }
         }
    }


  if(strcmp(buf,"pinoff\n")==0){       
  UART_putString((uint8_t*)"pin 7 OFF \n");
 OCR4BL=255;
}


  }
}
