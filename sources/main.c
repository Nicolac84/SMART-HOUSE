#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>

#include "communication.h"
#include "serial.h"
#include "pwm.h"
#include "analog.h"
#include "digitin.h"
#include "status.h"

#define MAIN_LOOP_DELAY 10

int main(void) {
  sei();

  // Initialize all stuff
  status_load();
  serial_init();
  pwm_init();
  digit_init();
  analog_init();

  // Blink LED connected to port D30
  DDRC |= (1 << 7);
  PORTC |= (1 << 7);
  for (uint8_t i=1; i <= 3; ++i) {
    _delay_ms(150);
    PORTC &= ~(1 << 7);
    _delay_ms(150);
    PORTC |= (1 << 7);
  }

  // Main application loop
  while (1) {
    communication_handler();
    _delay_ms(MAIN_LOOP_DELAY);
  }
}
