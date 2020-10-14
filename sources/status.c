#include "status.h"
#include <avr/eeprom.h>
#include <string.h>

status_t EEMEM _status_eeprom = (status_t) {
  .digital_in = {
    "digitin0", "digitin1", "digitin2", "digitin3",
    "digitin4", "digitin5", "digitin6", "digitin7"
  },
  .analog = {
   "analog0", "analog1", "analog2", "analog3",
    "analog4", "analog5", "analog6", "analog7"
  },
  .pwm = {
    "pwm0", "pwm1", "pwm2", "pwm3",
    "pwm4", "pwm5", "pwm6", "pwm7"
  }
};

status_t status = { 0 };
status_t *status_eeprom = &_status_eeprom;


void status_save(void){
  eeprom_busy_wait();
  eeprom_update_block(&status, status_eeprom, sizeof(status_t));
  eeprom_busy_wait();
}


void status_load(void){
  eeprom_busy_wait();
  eeprom_read_block(&status, status_eeprom, sizeof(status_t));
  eeprom_busy_wait();
}


uint8_t status_digitalin_pin_set(uint8_t pin, const uint8_t * name){
  strncpy((char*)(status.digital_in[pin]), (const char*) name, PIN_MAX_LEN);
  status.digital_in[pin][PIN_MAX_LEN]= '\0';
  return 0;
}

uint8_t status_analog_pin_set(uint8_t pin, const uint8_t * name){
  strncpy((char*) status.analog[pin], (const char*) name, PIN_MAX_LEN);
  status.analog[pin][PIN_MAX_LEN]= '\0';
  return 0;
}

uint8_t status_pwm_pin_set(uint8_t pin, const uint8_t * name){
  strncpy((char*) status.pwm[pin], (const char*) name, PIN_MAX_LEN);
  status.pwm[pin][PIN_MAX_LEN]= '\0';
  return 0;
}
