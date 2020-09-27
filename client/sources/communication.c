#include <string.h>
#include "communication.h"
#include "serial.h"


uint8_t packet_create(packet_t *p, uint8_t pin, uint8_t operation, 
    uint8_t len, const void *body){

  if (pin > COM_DIGITIN7) return 1;
  if (operation > COM_OP_NAK) return 1;
  p->header = (pin << 12) | (operation << 8) | len;

  uint8_t i;
  for (i=0; i < len; i++){
    p->body[i] = ((uint8_t*) body)[i];
  }
  checksum_t cksum = packet_checksum(p);
  memcpy(p->body+len, &cksum, sizeof(cksum));

  return 0;
}


// Computes and returns the uncomplemented checksum
static checksum_t _checksum(const packet_t *p) {
  uint8_t len = packet_get_size(p) + sizeof(p->header);
  uint8_t *data = (uint8_t*) p;

  uint32_t checksum = 0xffff;

  for(uint8_t i=0; i+1 < len; i+=2) {
    uint16_t word;
    memcpy(&word, data+i, 2);
    checksum += word;
    if(checksum > 0xffff){
      checksum -=0xffff ;
    }
  }

  if(len&1){
    uint16_t word=0;
    memcpy(&word, data+len-1, 1);
    checksum += word;
    if(checksum > 0xffff) {
      checksum -= 0xffff;
    }
  }

  return checksum;
}


// Compute the complemented checksum
checksum_t packet_checksum (const packet_t *p){
  checksum_t checksum = _checksum(p);
  return ~checksum; 
}


// Returns 0 on sane packet, 1 on corrupted packet
uint8_t packet_validate(const packet_t *p) {
  checksum_t checksum = _checksum(p);

  uint8_t len = packet_get_size(p);
  checksum_t packet_cksum;
  memcpy(&packet_cksum, p->body + len, sizeof(checksum_t));

  checksum += packet_cksum;
  if(checksum > 0xffff) checksum -= 0xffff;

  return (~checksum) ? 1 : 0;
}


void communication_send(uint8_t pin, uint8_t operation, uint8_t len,
    const void *body) {
  packet_t p[1];
  packet_create(p, pin, operation, len, body);
  serial_tx((uint8_t*) p, len + 2 + sizeof(checksum_t));
}


void communication_recv(packet_t *p) {
  serial_rx((uint8_t*) p, packet_get_size(p) + 2 + sizeof(checksum_t));
}



// TODO: Move in shell.c

uint8_t get_pin_name(uint8_t pin, char *dest) {
  if (!dest || pin <= COM_PIN_NULL || pin > COM_DIGITIN7) return 1;

  communication_send(pin, COM_OP_GET_NAME, 0, 0);

  packet_t p[1];
  communication_recv(p);
  uint8_t len = packet_get_size(p);
  memcpy(dest, p->body, len);
  dest[len] = '\0';

  return 0;
}

uint8_t set_pin_name(uint8_t pin, const char *name) {
  if (!name || pin <= COM_PIN_NULL || pin > COM_DIGITIN7) return 1;

  communication_send(pin, COM_OP_SET_NAME, strlen(name), name);

  packet_t p[1];
  communication_recv(p);

  return 0;
}
