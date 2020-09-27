// Smart House
// Config status module
#include <string.h>
#include "config.h"
#include "communication.h"

// Current configuration instance
config_t config;


// Fetch the current config from the AVR
uint8_t config_fetch(void) {
  for (com_pin_t pin=COM_PWM0; pin <= COM_PWM7; ++pin)
    get_pin_name(pin, config.pwm[pin - COM_PWM0]);
  for (com_pin_t pin=COM_ANALOG0; pin <= COM_ANALOG7; ++pin)
    get_pin_name(pin, config.pwm[pin - COM_ANALOG0]);
  for (com_pin_t pin=COM_DIGITIN0; pin <= COM_DIGITIN7; ++pin)
    get_pin_name(pin, config.pwm[pin - COM_DIGITIN0]);
  return 0;
}


// Returns the pin number for the pin named 'pin_name'
com_pin_t config_get_pin(const char *pin_name) {
  if (!pin_name) return COM_PIN_NULL;

  for (unsigned char pin=0; pin < 7; ++pin)
    if (strcmp(pin_name, config.pwm[pin]) == 0) return pin + COM_PWM0;
  for (unsigned char pin=0; pin < 7; ++pin)
    if (strcmp(pin_name, config.analog[pin]) == 0) return pin + COM_ANALOG0;
  for (unsigned char pin=0; pin < 7; ++pin)
    if (strcmp(pin_name, config.digital_in[pin]) == 0) return pin + COM_DIGITIN0;

  return COM_PIN_NULL;
}


// Set pin name of 'pin' to 'pin_name'
uint8_t config_set_pin(com_pin_t pin, const char *pin_name) {
  if (!pin_name || pin <= COM_PIN_NULL || pin > COM_DIGITIN7) return 1;

  if (pin >= COM_PWM0 && pin <= COM_PWM7)
    strcpy(config.pwm[pin-COM_PWM0], pin_name);
  if (pin >= COM_ANALOG0 && pin <= COM_ANALOG7)
    strcpy(config.analog[pin-COM_ANALOG0], pin_name);
  if (pin >= COM_DIGITIN0 && pin <= COM_DIGITIN7)
    strcpy(config.digital_in[pin-COM_DIGITIN0], pin_name);

  return 0;
}


// Get pin name
const char *config_get_pin_name(com_pin_t pin) {
  const char *name = NULL;

  if (pin >= COM_PWM0 && pin <= COM_PWM7)
    name = config.pwm[pin-COM_PWM0];
  if (pin >= COM_ANALOG0 && pin <= COM_ANALOG7)
    name = config.analog[pin-COM_ANALOG0];
  if (pin >= COM_DIGITIN0 && pin <= COM_DIGITIN7)
    name = config.digital_in[pin-COM_DIGITIN0];

  return (name && *name) ? name : NULL;
}
