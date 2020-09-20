#include <stdio.h>
#include "communication.h"
#include "serial.h"



typedef enum STATE {
	STATE_LISTEN,
	STATE_HEADER,
	STATE_BODY,
	STATE_CKSUM,
	STATE_UNROLL,
	STATE_EXECUTE,
	STATE_ACK,
	STATE_NAK

}state_t;



typedef void (*operation_f)(const packet_t*, uint8_t pin);

static _op_pin_null(const packet_t *p, uint8_t pin) {
return STATE_NAK;
}

static _op_on(const packet_t *p, uint8_t pin) {
if( pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
pwm_on(pinmap(pin));
return STATE_ACK
}

static _op_off(const packet_t *p, uint8_t pin) {
if( pin < COM_PWM0 || pin > COM_PWM7) return STATE_NAK;
pwm_off(pinmap(pin));

return STATE_ACK
}

static _op_set_val(const packet_t *p, uint8_t pin) {
if( pin < COM_PWM0 || pin > COM_PWM7)  return STATE_NAK;
  
   pwm_regulate(pinmap(pin), p->body[0]);
return STATE_ACK;

}

static _op_get_name(const packet_t *p, uint8_t pin) {
} if (pin < COM_PWM0 || > COM_DIGITIN7) return STATE_NAK;

 if(pin >= COM_PWM0 && pin <= COM_PWM7) {

    comunication_send(pin, COM_OP_ACK, strlen(status.pwm[pin])+1, status.pmw[pin]);

}

 if(pin >= COM_ANALOG0 && pin <= COM_ANALOG7) {
comunication_send(pin, COM_OP_ACK, strlen(status.analog[pin])+1, status.analog[pin]);
    
}

 if(pin >= COM_DIGITIN0 && pin <= COM_DIGITIN7) {
comunication_send(pin, COM_OP_ACK, strlen(status.digital_in[pin])+1, status.digital_in[pin]);

}
return STATUS_LISTEN;
}

static _op_get_val(const packet_t *p, uint8_t pin) {
 if (pin < COM_PWM0 || > COM_DIGITIN7) return STATE_NAK;
 

 if(pin >= COM_PWM0 && pin <= COM_PWM7) {

    comunication_send(pin, COM_OP_ACK, 1,pwm_read(pinmap(pin) ));

}

 if(pin >= COM_ANALOG0 && pin <= COM_ANALOG7) {
comunication_send(pin, COM_OP_ACK, 1, analog_convert(pinmap(pin)));
    
}

 if(pin >= COM_DIGITIN0 && pin <= COM_DIGITIN7) {
comunication_send(pin, COM_OP_ACK, 1 , digit_read(pinmap(pin)));

}
return STATUS_LISTEN;
}



static _op_set_name(const packet_t *p, uint8_t pin) {
 if (pin < COM_PWM0 || > COM_DIGITIN7) return STATE_NAK;

 if(pin >= COM_PWM0 && pin <= COM_PWM7) {

    status_pwm_pin_set(pinmap(pin),p->body);

}

 if(pin >= COM_ANALOG0 && pin <= COM_ANALOG7) {

    status_analog_pin_set(pinmap(pin),p->body);
}

 if(pin >= COM_DIGITIN0 && pin <= COM_DIGITIN7) {

    status_digitalin_pin_set(pinmap(pin),p->body);
}
return STATUS_ACK
}
static _op_ack(const packet_t *p, uint8_t pin) {

}

static _op_nak(const packet_t *p, uint8_t pin) {
}


static operation_f op_table[] = {
  _op_null, _op_on, _op_off, _op_get_val, _op_set_val, _op_get_name,
  _op_set_name, _op_ack, _op_nak,
};


// Map communication pin values to effective values
static uint8_t pinmap(uint8_t com) {
  if (com == COM_PIN_NULL) return 0;
  if (com >= COM_PWM0 && com <= COM_PWM7)
    return com - COM_PWM0;
  if (com >= COM_ANALOG0 && com <= COM_ANALOG7)
    return com - COM_ANALOG0;
  if (com >= COM_DIGITIN0 && com <= COM_DIGITIN7)
    return com - COM_DIGITIN0;
}


void main(void){
  static state_t state = STATE_LISTEN;

  packet_t packet;
  uint8_t *_packet = (uint8_t*) &packet;
  uint8_t received = 0;

  uint8_t port = COM_PIN_NULL;
  uint8_t operation = COM_OP_NULL;

  packet_t tx_packet;

  uint8_t exit_loop = 0;
  while (!exit_loop) {
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
        uint8_t to_recv = packet_get_size(&packet) + sizeof(checksum_t);
        serial_rx(_packet + received, to_recv);
        received += to_recv;
        ++state;
        break;
      case STATE_CKSUM:
        state = packet_validate(&packet) ? STATE_UNROLL : STATE_NAK;
        break;
      case STATE_UNROLL:
        port = packet_get_pin(&packet);
        operation = packet_get_operation(&packet);
        ++state;
        break;
      case STATE_EXECUTE:
        op_table[operation](&packet, port);
        ++state;
        break;
      case STATE_ACK:
        packet_create(&packet_tx, COM_PIN_NULL, COM_OP_ACK, 0, NULL);
        serial_tx((void*) &packet_tx, 2 + sizeof(checksum_t));
        state = STATE_LISTEN;
        break;
      case STATE_NAK:
        packet_create(&packet_tx, COM_PIN_NULL, COM_OP_NAK, 0, NULL);
        serial_tx((void*) &packet_tx, 2 + sizeof(checksum_t));
        state = STATE_LISTEN;
        break;
    }
  }
}

