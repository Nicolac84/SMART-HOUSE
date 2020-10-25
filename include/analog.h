// Smart House - Nicola Colao
// Analog I/O using AVR embedded ADC
#ifndef __ANALOG_MODULE_H
#define __ANALOG_MODULE_H
#include <stdint.h>

// Configurable ADC analog pin used for the LM35
typedef enum ADC_PIN_E {
  A0 = 0, A1, A2, A3, A4, A5, A6, A7
} adc_pin_t;


// Initialization
void analog_init(void);

// Start a conversion and enter in ADC Noise Reduction sleep mode until the
// conversion completes
uint8_t analog_convert(uint8_t adc_pin);

#endif  // __ANALOG_MODULE_H
