// Smart House
// Config status module
#include <string.h>
#include "config.h"
#include "communication.h"

// Current configuration instance
config_t config;


// Fetch the current config from the AVR
// Returns 0 on success, 1 on failure
static inline int _config_fetch() {
  for (uint8_t pin=0; pin < 8; ++pin)
    if (communication_get_pin_name(pin+COM_PWM0, config.pwm[pin]))
      return 1;
  for (uint8_t pin=0; pin < 8; ++pin)
    if (communication_get_pin_name(pin+COM_ANALOG0, config.analog[pin]))
      return 1;
  for (uint8_t pin=0; pin < 8; ++pin)
    if (communication_get_pin_name(pin+COM_DIGITIN0, config.digital_in[pin]))
      return 1;
  return 0;
}

uint8_t config_fetch(void) {
  if (_config_fetch() == 0) return 0;
  fprintf(stderr, "Error while fetching configuration");
  return 1;
}


// Returns the pin number for the pin named 'pin_name'
com_pin_t config_get_pin(const char *pin_name) {
  if (!pin_name) return COM_PIN_NULL;

  for (unsigned char pin=0; pin < 8; ++pin)
    if (strcmp(pin_name, config.pwm[pin]) == 0)
      return pin + COM_PWM0;
  for (unsigned char pin=0; pin < 8; ++pin)
    if (strcmp(pin_name, config.analog[pin]) == 0)
      return pin + COM_ANALOG0;
  for (unsigned char pin=0; pin < 8; ++pin)
    if (strcmp(pin_name, config.digital_in[pin]) == 0)
      return pin + COM_DIGITIN0;

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
