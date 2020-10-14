#include <string.h>
#include "communication.h"
#include "serial.h"
#include "status.h"
#include "pwm.h"
#include "digitin.h"
#include "analog.h"

// Map communication pin values to effective values
static uint8_t pinmap(uint8_t com);

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
  return _checksum(p, packet_get_size(p)) ? 1 : 0;
}


void communication_send(uint8_t pin, uint8_t operation, uint8_t body_size,
    const void *body) {
  packet_t p[1];
  packet_create(p, pin, operation, body_size, body);
  serial_tx((uint8_t*) p, packet_get_size(p));
}


// Communication handler
typedef enum STATE_E {
  STATE_LISTEN,
  STATE_HEADER,
  STATE_BODY,
  STATE_CKSUM,
  STATE_UNROLL,
  STATE_EXECUTE,
  STATE_ACK,
  STATE_NAK
} state_t;

// Callback for communication operation
typedef state_t (*operation_f)(const packet_t*, uint8_t pin);

// Do nothing (COM_OP_NULL is unexpected)
static state_t _op_null(const packet_t *p, uint8_t pin) {
  return STATE_NAK;
}

// Turn a PWM pin ON
static state_t _op_on(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
  pwm_on(pinmap(pin));
  return STATE_ACK;
}

// Turn a PWM pin OFF
static state_t _op_off(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
  pwm_off(pinmap(pin));
  return STATE_ACK;
}

// Get the current value for a pin
static state_t _op_get_val(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_DIGITIN7) return STATE_NAK;
  uint8_t real = pinmap(pin);
  uint8_t val;

  if (pin >= COM_PWM0 && pin <= COM_PWM7)
    val = pwm_read(real);
  else if (pin >= COM_ANALOG0 && pin <= COM_ANALOG7)
    val = analog_convert(real);
  else
    val = digit_read(real);

  communication_send(pin, COM_OP_ACK, 1, &val);
  return STATE_LISTEN;
}

// Set the current value for a PWM pin
static state_t _op_set_val(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
  pwm_regulate(pinmap(pin), p->body[0]);
  return STATE_ACK;
}

// Get the name for a pin
static state_t _op_get_name(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_DIGITIN7) return STATE_NAK;
  uint8_t real = pinmap(pin);

  if (pin >= COM_PWM0 && pin <= COM_PWM7)
    communication_send(pin, COM_OP_ACK, strlen(
          (const char*)(status.pwm[real]))+1, status.pwm[real]);
  else if (pin >= COM_ANALOG0 && pin <= COM_ANALOG7)
    communication_send(pin, COM_OP_ACK, strlen(
          (const char*) status.analog[real])+1, status.analog[real]);
  else
    communication_send(pin, COM_OP_ACK, strlen(
          (const char*) status.digital_in[real])+1, status.digital_in[real]);

  return STATE_LISTEN;
}

// Setthe name for a pin
static state_t _op_set_name(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_DIGITIN7) return STATE_NAK;
  uint8_t real = pinmap(pin);

  if (pin >= COM_PWM0 && pin <= COM_PWM7)
    status_pwm_pin_set(real, p->body);
  else if (pin >= COM_ANALOG0 && pin <= COM_ANALOG7)
    status_analog_pin_set(real, p->body);
  else status_digitalin_pin_set(real, p->body);

  return STATE_ACK;
}

// Save status to the EEPROM
static state_t _op_save_status(const packet_t *p, uint8_t pin) {
  status_save();
  return STATE_ACK;
}

// ACK and NAK are not expected, do nothing
static state_t _op_ack(const packet_t *p, uint8_t pin) { return STATE_LISTEN; }
static state_t _op_nak(const packet_t *p, uint8_t pin) { return STATE_LISTEN; }


// Operation table
static operation_f op_table[] = {
  _op_null,
  _op_ack,
  _op_nak,
  _op_on,
  _op_off,
  _op_get_val,
  _op_set_val,
  _op_get_name,
  _op_set_name,
  _op_save_status
};


// Map communication pin values to effective values
static uint8_t pinmap(uint8_t com) {
  if (com < COM_PWM0 || com > COM_DIGITIN7) return 0;
  if (com >= COM_PWM0 && com <= COM_PWM7)
    return com - COM_PWM0;
  else if (com >= COM_ANALOG0 && com <= COM_ANALOG7)
    return com - COM_ANALOG0;
  else
    return com - COM_DIGITIN0;
}


// Communication handler routine
void communication_handler(void) {
  static state_t state = STATE_LISTEN;

  packet_t rx[1];
  uint8_t *_rx = (uint8_t*) rx;

  uint8_t port = COM_PIN_NULL;
  uint8_t operation = COM_OP_NULL;

  uint8_t received = 0;
  uint8_t to_recv;

  // Main handler loop
  while (1) {
    switch (state) {
      case STATE_LISTEN:  // Check for available data
        if (serial_available()) ++state;
        else return;
        break;
      case STATE_HEADER:  // Receive packet header
        serial_rx(_rx, sizeof(header_t));
        received = sizeof(header_t);
        ++state;
        break;
      case STATE_BODY:  // Receive packet body and checksum
        to_recv = packet_get_size(rx) - sizeof(header_t);
        serial_rx(_rx + received, to_recv);
        received += to_recv;
        ++state;
        break;
      case STATE_CKSUM:  // Check packet sanity
        state = packet_validate(rx) ? STATE_NAK : STATE_UNROLL;
        break;
      case STATE_UNROLL:  // Extract instructions from packet
        port = packet_get_pin(rx);
        operation = packet_get_operation(rx);
        ++state;
        break;
      case STATE_EXECUTE:  // Execute the command given by the client
        state = op_table[operation](rx, port);
        break;
      case STATE_ACK:  // Acknowledge an operation
        communication_send(COM_PIN_NULL, COM_OP_ACK, 0, NULL);
        state = STATE_LISTEN;
        break;
      case STATE_NAK:  // Raise an error to the client
        communication_send(COM_PIN_NULL, COM_OP_NAK, 0, NULL);
        state = STATE_LISTEN;
        break;
    }
  }
}
