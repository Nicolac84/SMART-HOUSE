// Smart House - PWM module test
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pwm.h"
#include "serial.h"

#define WELCOME_MSG "Inizializzata tutta robba\n"

int main(void) {
  sei();
  serial_init();
  pwm_init();

  serial_tx(WELCOME_MSG, sizeof(WELCOME_MSG) - 1);

  pwm_on(PWM5);

  uint8_t current = 0;
  while (1) {
    _delay_ms(1000);
    current = (current + 10) % 245;
    pwm_regulate(PWM5, current);
    serial_tx(&current, 1);
  }
}
