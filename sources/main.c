#include <avr/interrupt.h>
#include <util/delay.h>

#include "communication.h"
#include "serial.h"
#include "pwm.h"
#include "analog.h"
#include "digitin.h"
#include "status.h"

#define READY_MSG "Ciaone\n"
static inline void ciaone(void) {
  serial_tx(READY_MSG, 7);
}

int main(void) {
  sei();

  // Initialize all stuff
  serial_init();
  ciaone();

  pwm_init();
  digit_init();
  analog_init();
  ciaone();

  // Load status from the board's EEPROM
  status_load();
  ciaone();

  // Main application loop
  while (1) {
    communication_handler();
    _delay_ms(500);
  }
}
