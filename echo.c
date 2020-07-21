#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>

#include <avr_common/uart.h>

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


//fun per attivare  le porte analogiche
void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0);
 
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}



#define MAX_BUF 256
int main(void){

 printf_init(); 

 adc_init();

// we will use timer 1
  TCCR1A=TCCRA_MASK;
  TCCR1B=TCCRB_MASK;
  // contatori pin 2 ,3 e 5
  TCCR3A=TCCRA3_MASK;
  TCCR3B=TCCRB3_MASK;
 
    // contatori pin 6, 7 e 8
  TCCR4A=TCCRA4_MASK;
  TCCR4B=TCCRB4_MASK;

  const uint8_t mask=(1<<5)|(1<<6)|(1<<4)|(1<<3);
  // we configure the pin as output 11, 12
  DDRB = mask; //mask;
// maschera attivazione pin 2 e 3,5
  DDRE = mask;
// maschera attivazione pin 6,7 e 8
  DDRH = mask;


//funzione blink
void blink(uint8_t * lumb){
  uint8_t intensity=255;
  
  while(1){
    // we write on the output compare register a value
    // that will be proportional to the opposite of the
    // duty_cycle
  *lumb = intensity;
    _delay_ms(100); // from delay.h, wait 1 sec
    intensity-=5;
  }

}


  UART_init();
  UART_putString((uint8_t*)"READY\n  ");
  UART_putString((uint8_t*)"MENU: allpinon  -  allpinoff  -  pinon \n  ");

  uint8_t buf[MAX_BUF];


// funzione seletiva pin  dimmer 
void PinONsel (uint8_t pin, uint8_t lum){
       switch (pin){
         case 1:  OCR1BL=lum; UART_putString((uint8_t*)"valore settato per pin 12"); break;
         case 2:  OCR3BL=lum; UART_putString((uint8_t*)"valore settato per pin 2 "); break;
         case 3:  OCR3CL=lum; UART_putString((uint8_t*)"valore settato per pin 3 "); break;
         case 4:  OCR1AL=lum; UART_putString((uint8_t*)"valore settato per pin 11"); break;
         case 5:  OCR3AL=lum; UART_putString((uint8_t*)"valore settato per pin 5 "); break;
         case 6:  OCR4AL=lum; UART_putString((uint8_t*)"valore settato per pin 6 "); break;
         case 7:  OCR4BL=lum; UART_putString((uint8_t*)"valore settato per pin 7 "); break;
         case 8:  OCR4CL=lum; UART_putString((uint8_t*)"valore settato per pin 8 "); break;
         default:  break;
         }
}


// funzione tutti i pin accesi a luminosita uguale
void AllPin (uint8_t lum){
        OCR1AL=lum;  //11
        OCR1BL=lum;  //12
        OCR3AL=lum;  //5
        OCR3BL=lum;  //2
        OCR3CL=lum;  //3
        OCR4AL=lum;  //6
        OCR4BL=lum;  //7
        OCR4CL=lum;  //8
}

//prova analogici



uint16_t adc_read(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);
 
  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));
 
  return (ADC);
}

 while(1) {

    
uint16_t p=adc_read(5);
uint16_t p2=adc_read(7);
  
   printf("switch  %d,%d\n",p,p2);
 // UART_putString((uint16_t*)"valore %u \n",p); questa nn va e nn so il perche!
    
    UART_getString(buf);
    UART_putString((uint8_t*)"received\n");
    UART_putString(buf);

   // comando per accendere tutti i pin da 1 a 8
   if(strcmp("allpinon\n",buf)==0){ 
     AllPin(1);
    UART_putString((uint8_t*)"tutto acceso =) \n");
    }

   // dimmer selettivo
   if(strcmp("pinon\n",buf)==0){
        UART_putString((uint8_t*)"Scegli un pin da 1-8 \n");
        UART_getString(buf);
        uint8_t pin=atoi(buf);
        if((pin <9) && (pin >0)){
           UART_putString((uint8_t*)"inserisci valore 0-255 per dimmerare per il pin \n");
           UART_getString(buf);
           uint8_t  lum=atoi(buf);
            PinONsel(pin,lum);

        }
        else {  UART_putString((uint8_t*)"PIN NON DISPONIBILE inserisci valore pin corretto \n");
         }
    }

    //Tutti i pin off
    if(strcmp(buf,"allpinoff\n")==0){
      AllPin(255);
      UART_putString((uint8_t*)"tutto spento \n");
    }

 }
}
