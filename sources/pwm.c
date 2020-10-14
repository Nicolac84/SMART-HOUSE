// Smart House - PWM module
#include <avr/io.h>
#include "pwm.h"

// Configure PWM timers | Fast PWM, 8 bit, non inverted
#define TCCR1A_MASK ((1 << WGM10) | (1 << COM1C0) | (1 << COM1C1) | (1 << COM1B1) | (1 << COM1B0) | (1 << COM1A1) | (1 << COM1A0))
#define TCCR1B_MASK ((1 << WGM12) | (1 << CS10))
#define TCCR3A_MASK ((1 << WGM30) | (1 << COM3C0) | (1 << COM3C1) | (1 << COM3B1) | (1 << COM3B0) | (1 << COM3A1) | (1 << COM3A0))
#define TCCR3B_MASK ((1 << WGM32) | (1 << CS30))
#define TCCR4A_MASK ((1 << WGM40) | (1 << COM4C0) | (1 << COM4C1) | (1 << COM4B1) | (1 << COM4B0) | (1 << COM4A1) | (1 << COM4A0))
#define TCCR4B_MASK ((1 << WGM42) | (1 << CS40))


// Initialize PWM module
void pwm_init(void) {
  // Set all PWM pins off
  DDRB = DDRE = DDRH = 0x00;

  // Configure timers
  TCCR1A = TCCR1A_MASK; // PWM pin 11
  TCCR1B = TCCR1B_MASK; // PWM pin 12
  TCCR3A = TCCR3A_MASK; // PWM pin 2 and 3
  TCCR3B = TCCR3B_MASK; // PWM pin 5
  TCCR4A = TCCR4A_MASK; // PWM pin 6 and 7
  TCCR4B = TCCR4B_MASK; // PWM pin 8
}


// Turn PWM pins on
// Returns 0 on success, 1 on failure
uint8_t pwm_on(uint8_t pwm_pin) {
  if (pwm_pin > PWM12) return 1;
  switch (pwm_pin) {
    case PWM2:  DDRE |= (1 << 4); break;
    case PWM3:  DDRE |= (1 << 5); break;
    case PWM5:  DDRE |= (1 << 3); break;
    case PWM6:  DDRH |= (1 << 3); break;
    case PWM7:  DDRH |= (1 << 4); break;
    case PWM8:  DDRH |= (1 << 5); break;
    case PWM11: DDRB |= (1 << 5); break;
    case PWM12: DDRB |= (1 << 6); break;
  }
  return 0;
}

// Turn PWM pins off
uint8_t pwm_off(uint8_t pwm_pin) {
  if (pwm_pin > PWM12) return 1;
  switch (pwm_pin) {
    case PWM2:  DDRE &= ~(1 << 4); break;
    case PWM3:  DDRE &= ~(1 << 5); break;
    case PWM5:  DDRE &= ~(1 << 3); break;
    case PWM6:  DDRH &= ~(1 << 3); break;
    case PWM7:  DDRH &= ~(1 << 4); break;
    case PWM8:  DDRH &= ~(1 << 5); break;
    case PWM11: DDRB &= ~(1 << 5); break;
    case PWM12: DDRB &= ~(1 << 6); break;
  }
  return 0;
}

// Regulate wave duty cicle
uint8_t pwm_regulate(uint8_t pwm_pin, uint8_t val) {
  if (pwm_pin > PWM12) return 1;
  switch (pwm_pin) {
    case PWM2:  OCR3BL = val; break;
    case PWM3:  OCR3CL = val; break;
    case PWM5:  OCR3AL = val; break;
    case PWM6:  OCR4AL = val; break;
    case PWM7:  OCR4BL = val; break;
    case PWM8:  OCR4CL = val; break;
    case PWM11: OCR1AL = val; break;
    case PWM12: OCR1BL = val; break;
  }
  return 0;
}

// Get the current duty cicle value for a PWM pin
uint8_t pwm_read(uint8_t pwm_pin) {
  if (pwm_pin > PWM12) return 0;
  switch (pwm_pin) {
    case PWM2:  return OCR3BL;
    case PWM3:  return OCR3CL;
    case PWM5:  return OCR3AL;
    case PWM6:  return OCR4AL;
    case PWM7:  return OCR4BL;
    case PWM8:  return OCR4CL;
    case PWM11: return OCR1AL;
    case PWM12: return OCR1BL;
  }
 return 0;
}
