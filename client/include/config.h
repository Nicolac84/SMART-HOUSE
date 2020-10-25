// Smart House - Nicola Colao
// Config status module
#ifndef __CONFIG_STATUS_H
#define __CONFIG_STATUS_H
#include "communication.h"

#define PIN_MAX_LEN 31

// Keep track of pin's names
typedef struct _config_s {
  char digital_in[8][PIN_MAX_LEN+1];
  char analog[8][PIN_MAX_LEN+1];
  char pwm[8][PIN_MAX_LEN+1];
} config_t;

// Global configuration instance
extern config_t config;

// Fetch the current config from the AVR
// Returns 0 on success, 1 on failure
uint8_t config_fetch(void);

// Returns the pin number for the pin named 'pin_name'
com_pin_t config_get_pin(const char *pin_name);

// Set pin name of 'pin' to 'pin_name'
uint8_t config_set_pin(com_pin_t pin, const char *pin_name);

// Get pin name
const char *config_get_pin_name(com_pin_t pin);

#endif
