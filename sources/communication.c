#include <string.h>
#include "communication.h"
#include "serial.h"
#include "status.h"
#include "pwm.h"
#include "digitin.h"
#include "analog.h"

// [AUX] Map communication pin values to effective values
static uint8_t pinmap(uint8_t com);

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
  packet_t p;
  packet_create(&p, pin, operation, len, body);
  serial_tx((uint8_t*) &p, len + 2 + sizeof(checksum_t));
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

typedef state_t (*operation_f)(const packet_t*, uint8_t pin);

// [AUX] Transmit current automa state
static inline void tx_state(state_t state);

static state_t _op_null(const packet_t *p, uint8_t pin) {
  return STATE_NAK;
}

static state_t _op_on(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
  pwm_on(pinmap(pin));
  return STATE_ACK;
}

static state_t _op_off(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
  pwm_off(pinmap(pin));
  return STATE_ACK;
}

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

static state_t _op_set_val(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
  pwm_regulate(pinmap(pin), p->body[0]);
  return STATE_ACK;
}

static state_t _op_get_name(const packet_t *p, uint8_t pin) {
  if (pin < COM_PWM0 || pin > COM_DIGITIN7) return STATE_NAK;
  uint8_t real = pinmap(pin);

  if (pin >= COM_PWM0 && pin <= COM_PWM7)
    communication_send(pin, COM_OP_ACK,
        strlen((const char*) status.pwm[real])+1, status.pwm[real]);
  else if (pin >= COM_ANALOG0 && pin <= COM_ANALOG7)
    communication_send(pin, COM_OP_ACK,
        strlen((const char*) status.analog[real])+1, status.analog[real]);
  else
    communication_send(pin, COM_OP_ACK,
        strlen((const char*) status.digital_in[real])+1, status.digital_in[real]);

  return STATE_LISTEN;
}

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

// ACK and NAK are not expected, do nothing
static state_t _op_ack(const packet_t *p, uint8_t pin) { return STATE_LISTEN; }
static state_t _op_nak(const packet_t *p, uint8_t pin) { return STATE_LISTEN; }


static operation_f op_table[] = {
  _op_null, _op_on, _op_off, _op_get_val, _op_set_val, _op_get_name,
  _op_set_name, _op_ack, _op_nak,
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


void communication_handler(void) {
  static state_t state = STATE_LISTEN;

  packet_t packet;
  uint8_t *_packet = (uint8_t*) &packet;
  uint8_t received = 0;

  uint8_t port = COM_PIN_NULL;
  uint8_t operation = COM_OP_NULL;

  packet_t tx_packet;

  uint8_t to_recv; // Auxiliary variable

  uint8_t exit_loop = 0;
  while (!exit_loop) {
    tx_state(state);
    switch (state) {
      case STATE_LISTEN:
        if (serial_available()) ++state;
        else return;
        break;
      case STATE_HEADER:
        serial_rx(_packet, 2);
        received += 2;
        ++state;
        break;
      case STATE_BODY:
        to_recv = packet_get_size(&packet) + sizeof(checksum_t);
        serial_rx(_packet + received, to_recv);
        received += to_recv;
        ++state;
        break;
      case STATE_CKSUM:
        //state = packet_validate(&packet) ? STATE_NAK : STATE_UNROLL;
        state = STATE_UNROLL; // DEBUG ONLY
        break;
      case STATE_UNROLL:
        port = packet_get_pin(&packet);
        operation = packet_get_operation(&packet);
        ++state;
        break;
      case STATE_EXECUTE:
        // TODO: Debug only, Remove
        serial_tx(&port, 1);
        serial_tx(&operation, 1);
        serial_tx(&packet, 4);
        while (1) ;

        state = op_table[operation](&packet, port);
        break;
      case STATE_ACK:
        packet_create(&tx_packet, COM_PIN_NULL, COM_OP_ACK, 0, NULL);
        serial_tx((void*) &tx_packet, 2 + sizeof(checksum_t));
        state = STATE_LISTEN;
        break;
      case STATE_NAK:
        packet_create(&tx_packet, COM_PIN_NULL, COM_OP_NAK, 0, NULL);
        serial_tx((void*) &tx_packet, 2 + sizeof(checksum_t));
        state = STATE_LISTEN;
        break;
    }
  }
}


static inline void tx_state(state_t state) {
  static const char state_str[][16] = {
    "STATE_LISTEN\n",
    "STATE_HEADER\n",
    "STATE_BODY\n",
    "STATE_CKSUM\n",
    "STATE_UNROLL\n",
    "STATE_EXECUTE\n",
    "STATE_ACK\n",
    "STATE_NAK\n"
  };
  serial_tx(state_str[state], strlen(state_str[state]));
}
