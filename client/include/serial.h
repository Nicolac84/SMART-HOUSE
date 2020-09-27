// Smart House
// Serial module
#ifndef __SERIAL_H
#define __SERIAL_H
#include <unistd.h>

#define BAUD_RATE B19200


int serial_open(const char *dev);

int serial_close(void);

int serial_rx(void *buf, unsigned size);

int serial_tx(const void *buf, unsigned size);

#endif
