// Analog I/O using AVR embedded ADC
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "analog.h"

static const uint8_t channel_selectors[8] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};


// Initialization
void analog_init(void) {
  ADMUX = (1 << REFS1) | (1 << REFS0); // 2.56V Internal Reference
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}


// Returns the registered temperature, or 0 on failure
uint8_t analog_convert(uint8_t adc_pin) {
  if (adc_pin > 7) return 0;

  ADMUX |= channel_selectors[adc_pin]; // Select channel

  // Busy wait until conversion complete
  ADCSRA |= (1 << ADSC);
  while (! (ADCSRA & (1 << ADIF))) ;
  uint8_t adc_result = ADC;

  return adc_result;
}
