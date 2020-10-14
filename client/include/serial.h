// Smart House
// Serial module
#ifndef __SERIAL_H
#define __SERIAL_H
#include <unistd.h>

#define BAUD_RATE B115200


// Open a serial device
// Returns 0 on success, 1 on failure
int serial_open(const char *dev_path);

// Close the currently open serial device
// Returns 0 on success, 1 on failure
int serial_close(void);

// Receive data from the serial port
// Returns the number of read bytes, or -1 on failure
int serial_rx(void *buf, unsigned size);

// Send data to the serial port
// Returns the number of written bytes, or -1 on failure
int serial_tx(const void *buf, unsigned size);

#endif
