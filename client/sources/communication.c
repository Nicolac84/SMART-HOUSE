#include <string.h>
#include <stdio.h>
#include "communication.h"
#include "serial.h"


// Create a packet
uint8_t packet_create(packet_t *p, uint8_t pin, uint8_t operation, 
    uint8_t body_size, const void *body){
  if (!p || pin > COM_DIGITIN7 || operation >= COM_OP_LIMIT ||
      body_size > BODY_MAX_LEN || (!body && body_size))
    return 1;

  p->header = packet_header(pin, operation, body_size);

  if (body) memcpy(p->body, body, body_size);

  checksum_t cksum = packet_checksum(p);
  memcpy(p->body + body_size, &cksum, sizeof(cksum));

  return 0;
}


// Computes and returns the uncomplemented checksum
static checksum_t _checksum(const void *data, uint8_t size) {
  checksum_t checksum = 0;
  for (uint8_t i=0; i < size; ++i)
    checksum ^= ((uint8_t*) data)[i];
  return checksum;
}


// Compute the complemented checksum
checksum_t packet_checksum (const packet_t *p){
  return _checksum(p, packet_get_size(p) - sizeof(checksum_t));
}


// Returns 0 on sane packet, 1 on corrupted packet
uint8_t packet_validate(const packet_t *p) {
  checksum_t checksum = _checksum(p, packet_get_size(p));
#ifdef DEBUG
  if (checksum) printf("Checksum mismatch: %hx\n", checksum);
#endif
  return checksum ? 1 : 0;
}


// Create in-place and send a packet
// Returns 0 on success, 1 on failure
int communication_send(uint8_t pin, uint8_t operation, uint8_t len,
    const void *body) {
  packet_t p[1];
  if (packet_create(p, pin, operation, len, body) != 0)
    return 1;

#ifdef DEBUG
  printf(
      "\nSending packet\n"
      "PIN: %hhu\n"
      "OP : %hhu\n"
      "LEN: %hhu\n"
      , packet_get_pin(p), packet_get_operation(p), packet_get_size(p)
      );
  printf("FIRST 16 BYTES: ");
  for (uint8_t i=0; i < 16; ++i)
    printf("%2hhx%c", ((uint8_t*) p)[i], i == 15 ? '\n' : ' ');
#endif

  return (serial_tx((void*) p, packet_get_size(p)) < 0) ? 1 : 0;
}


// Receive a packet
// Returns 0 on success, 1 on failure
int communication_recv(packet_t *p) {
  uint8_t *_p = (uint8_t*) p;
  int ret;

#ifdef DEBUG
  printf("\nReceiving packet\n");
#endif

  do {
    // Receive header first
    ret = serial_rx(_p, sizeof(header_t));
    if (ret < 0) break;

#ifdef DEBUG
  printf("Header received. Packet size is %hhu\n", packet_get_size(p));
#endif

    // Receive body and checksum
    // An error here could make communication hang from now on
    uint8_t remaining = packet_get_size(p) - sizeof(header_t);
    if (remaining - sizeof(checksum_t) > BODY_MAX_LEN) return -1;
    ret = serial_rx((uint8_t*) p->body, remaining);
  } while (0);

#ifdef DEBUG
  printf(
      "PIN: %hhu\n"
      "OP : %hhu\n"
      , packet_get_pin(p), packet_get_operation(p)
      );
  printf("FIRST 16 BYTES: ");
  for (uint8_t i=0; i < 16; ++i)
    printf("%2hhx%c", ((uint8_t*) p)[i], i == 15 ? '\n' : ' ');
#endif

  return ret < 0 ? 1 : packet_validate(p);
}


// Get pin names from the AVR board
uint8_t communication_get_pin_name(uint8_t pin, char *dest) {
  if (!dest || pin <= COM_PIN_NULL || pin > COM_DIGITIN7) return 1;

  com_handle_send(pin, COM_OP_GET_NAME, 0, NULL, 1, NULL, NULL);

  static packet_t p[1];
  com_handle_recv(p, 1, NULL, NULL);

#ifdef DEBUG
  printf("Received pin name: %s\n", (char*) p->body);
#endif

  uint8_t len = packet_get_size(p) - sizeof(header_t) - sizeof(checksum_t);
  memcpy(dest, p->body, len);
  dest[len] = '\0';

  return 0;
}


// Set pin names in the AVR board
uint8_t communication_set_pin_name(uint8_t pin, const char *name) {
  if (!name || pin <= COM_PIN_NULL || pin > COM_DIGITIN7) return 1;

  com_handle_send(pin, COM_OP_SET_NAME, strlen(name) + 1, name, 1, NULL, NULL);

  packet_t p[1];
  com_handle_recv(p, 1, NULL, NULL);

  return 0;
}
