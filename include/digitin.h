#ifndef __DIGITAL_INPUT_H
#define __DIGITAL_INPUT_H
#include <stdint.h>

typedef enum {
  PIN37, PIN36, PIN35, PIN34, PIN33, PIN32, PIN31, PIN30
} digit_pin_t;

// Initialize the digital ports
void digit_init(void);

// Read a digital port
uint8_t digit_read(uint8_t pin); 

#endif
