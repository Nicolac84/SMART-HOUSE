#include <util/delay.h>
#include <avr/interrupt.h>
#include "analog.h"
#include "serial.h"

#define WELCOME_MSG "Ciaone...\n"

int main(void) {
  sei();
  serial_init();
  analog_init();

  serial_tx(WELCOME_MSG, sizeof(WELCOME_MSG));
  while (1) {
    uint8_t result = analog_convert(0);
    _delay_ms(500);
    serial_tx(&result, 1);
  }
}
