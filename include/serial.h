// Smart House
// Serial Module
#ifndef __SERIAL_H
#define __SERIAL_H
#include <stdint.h>

#define BAUD_RATE 115200
#define MYUBRR (F_CPU / 8 / BAUD_RATE - 1)

// Initialize the serial port
void serial_init(void);

// Read at most 'size' bytes, returning the number of bytes effectively read
// Note that the function blocks until 'size' bytes are received
uint8_t serial_rx(void *buf, uint8_t size);

// Send 'size' bytes of data
uint8_t serial_tx(const void *buf, uint8_t size);

// Return the number of available bytes
uint8_t serial_available(void);

#endif
