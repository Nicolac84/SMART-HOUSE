#include "status.h"
#include <avr/eeprom.h>
#include <string.h>

status_t status = {0};
status_t EEMEM _status_eeprom;
status_t * status_eeprom = &_status_eeprom;


void status_save(void){
   eeprom_update_block (&status, status_eeprom,sizeof(status));
   eeprom_busy_wait();
}


void status_load(void){
   eeprom_read_block (&status, status_eeprom,sizeof(status));
   eeprom_busy_wait();
}


uint8_t status_digitalin_pin_set(uint8_t pin, char * name){
  strncpy(status.digital_in[pin] , name , PIN_MAX_LEN);
  status.digital_in[pin][PIN_MAX_LEN]= '\0';
  return 0;
}

uint8_t status_analog_pin_set(uint8_t pin, char * name){
  strncpy(status.analog[pin] , name , PIN_MAX_LEN);
  status.analog[pin][PIN_MAX_LEN]= '\0';
  return 0;
}

uint8_t status_pwm_pin_set(uint8_t pin, char * name){
  strncpy(status.pwm[pin] , name , PIN_MAX_LEN);
  status.pwm[pin][PIN_MAX_LEN]= '\0';
  return 0;
}

