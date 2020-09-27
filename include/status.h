#ifndef __STATUS_H
#define __STATUS_H

#include <stdint.h>
#define PIN_MAX_LEN 31


typedef struct {
  uint8_t digital_in[8][PIN_MAX_LEN+1];
  uint8_t analog[8][PIN_MAX_LEN+1];
  uint8_t pwm[8][PIN_MAX_LEN+1];
} status_t;


extern status_t status;

void status_save(void);
void status_load(void);
uint8_t status_digitalin_pin_set(uint8_t pin, const uint8_t * name);
uint8_t status_analog_pin_set(uint8_t pin, const uint8_t * name);
uint8_t status_pwm_pin_set(uint8_t pin, const uint8_t * name);
#endif
