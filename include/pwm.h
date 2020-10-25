// Smart House - Nicola Colao
// PWM output port handling module
#ifndef __PWM_MODULE_H
#define __PWM_MODULE_H
#include <stdint.h>

typedef enum PWM_E {
  PWM2, PWM3, PWM5, PWM6, PWM7, PWM8, PWM11, PWM12
} pwm_t;


// Initialize PWM module
void pwm_init(void);

// Turn PWM pins on/off
// Return 0 on success, 1 on failure
uint8_t pwm_on(uint8_t pwm_pin);
uint8_t pwm_off(uint8_t pwm_pin);

// Regulate wave duty cicle
uint8_t pwm_regulate(uint8_t pwm_pin, uint8_t val);

// Get the current duty cicle value for a PWM pin
uint8_t pwm_read(uint8_t pin);

#endif	// __PWM_MODULE_H
