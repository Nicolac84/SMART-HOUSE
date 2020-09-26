#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H
#include <stdint.h>

#define BODY_MAX_LEN 32

typedef enum COM_PIN_E {
  COM_PIN_NULL, // Something is not right if you get this
  COM_PWM0, COM_PWM1, COM_PWM2, COM_PWM3,
  COM_PWM4, COM_PWM5, COM_PWM6, COM_PWM7,
  COM_ANALOG0, COM_ANALOG1, COM_ANALOG2, COM_ANALOG3,
  COM_ANALOG4, COM_ANALOG5, COM_ANALOG6, COM_ANALOG7,
  COM_DIGITIN0, COM_DIGITIN1, COM_DIGITIN2, COM_DIGITIN3,
  COM_DIGITIN4, COM_DIGITIN5, COM_DIGITIN6, COM_DIGITIN7
} com_pin_t;

typedef enum COM_OPERATION_E {
  COM_OP_NULL, // Something is not right if you get this
  COM_OP_ON,
  COM_OP_OFF,
  COM_OP_GET_VAL,  // For PWM ports
  COM_OP_SET_VAL,  // For PWM ports
  COM_OP_GET_NAME,
  COM_OP_SET_NAME,
  COM_OP_ACK,
  COM_OP_NAK
} com_operation_t;


// -------------------
//  Packet Interface
// -------------------

typedef uint16_t header_t;
typedef uint16_t checksum_t;

typedef struct _packet_s {
  header_t header;
  uint8_t body[BODY_MAX_LEN+2]; // Body + Checksum
} packet_t;

// Get pin number
inline uint8_t packet_get_pin(const packet_t *p) {
  return p ? (p->header & 0xF000) >> 12 : COM_PIN_NULL;
}

// Get operation type
inline uint8_t packet_get_operation(const packet_t *p) {
  return p ? (p->header & 0x0F00) >> 8  : COM_OP_NULL;
}

// Get packet body size
inline uint8_t packet_get_size(const packet_t *p) {
  return p ? (p->header & 0x00FF) : 0;
}

// Create a packet
uint8_t packet_create(packet_t *p, uint8_t pin, uint8_t operation,
    uint8_t len, const void *body);

// Compute packet integrity checksum
checksum_t packet_checksum(const packet_t *p);

// Returns 0 on sane packet, 1 on corrupted packet
uint8_t packet_validate(const packet_t *p);


// -------------------------
//  Communication Interface
// -------------------------

void communication_send(uint8_t pin, uint8_t operation, uint8_t len,
    const void *body);

void communication_handler(void);

#endif	// __COMMUNICATION_H
