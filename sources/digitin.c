#include <avr/io.h>
#include "digitin.h"

// Initialize the digital ports
void digit_init(void) {
  DDRC = 0xFF;
  PORTC = 0xFF;
}

// Read a digital port
uint8_t digit_read(uint8_t pin) {
  if (pin > PIN30) return 2;
  return (PINC & (1 << pin)) >> pin;
}
