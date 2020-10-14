// Smart House
// Serial module
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "ringbuffer.h"
#include "serial.h"

#define RX_BUFFER_SIZE 128
#define TX_BUFFER_SIZE 128

// RX variables
static volatile uint8_t _rx_buffer[RX_BUFFER_SIZE];
static volatile ringbuf_t rx_buffer[1];

// TX variables
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_to_trans;
static volatile uint8_t tx_transmitted;

// Serial interrupts ON/OFF
#define rx_int_on()  do { UCSR0B |=   1 << RXCIE0 ; } while (0)
#define tx_int_on()  do { UCSR0B |=   1 << TXCIE0 ; } while (0)
#define rx_int_off() do { UCSR0B &= ~(1 << RXCIE0); } while (0)
#define tx_int_off() do { UCSR0B &= ~(1 << TXCIE0); } while (0)

void serial_init(void) {
  // RX buffer initialization
  ringbuffer_new((ringbuf_t*) rx_buffer,
      (uint8_t*) _rx_buffer, RX_BUFFER_SIZE);

  // Baud Rate
  UBRR0H = (uint8_t) (MYUBRR >> 8);
  UBRR0L = (uint8_t) MYUBRR;

  // 8 Bits communication and enable duplex communication
  UCSR0A = (1 << U2X0); // Double speed
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
  UCSR0B = (1 << TXEN0) | (1 << RXEN0);

  // Start receiving
  rx_int_on();
}


ISR(USART0_RX_vect) { // RX ISR
  ringbuffer_push((ringbuf_t*) rx_buffer, UDR0);
}

ISR(USART0_TX_vect) { // TX ISR
  ++tx_transmitted;
  if (tx_transmitted < tx_to_trans)
    UDR0 = tx_buffer[tx_transmitted];
}


// Read at most 'size' bytes, returning the number of bytes effectively read
// Note that the function blocks until 'size' bytes are received
uint8_t serial_rx(void *buf, uint8_t size) {
  if (!buf) return 0;

  for (uint8_t n=0; n < size; ++n)
    if (!ringbuffer_empty((ringbuf_t*) rx_buffer))
      ringbuffer_pop((ringbuf_t*) rx_buffer, buf + n);

  return size;
}


// Send 'size' bytes of data
uint8_t serial_tx(const void *buf, uint8_t size) {
  if (!buf || !size || size > TX_BUFFER_SIZE) return 0;

  tx_to_trans = size;
  tx_transmitted = 0;
  for (int i=0; i < size; ++i)
    tx_buffer[i] = ((uint8_t*) buf)[i];

  tx_int_on();
  UDR0 = tx_buffer[0];

  while (tx_to_trans != tx_transmitted) ;
  return size;
}


// Return the number of available bytes
uint8_t serial_available(void) {
  return ringbuffer_used((ringbuf_t*) rx_buffer);
}
