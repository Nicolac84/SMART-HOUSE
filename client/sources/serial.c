// Smart House
// Serial module
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

#include "serial.h"

// File descriptor for the current device
static int dev_fd = -1;
 
// Is the current device (if any) open and valid?
static inline int valid_dev(void) {
  return dev_fd > 2 && isatty(dev_fd) ? 1 : 0;
}


// Open a serial device
// Returns 0 on success, 1 on failure
int serial_open(const char *dev) {
  if (!dev || *dev == '\0') return 1;

  // Open the device file
  if ((dev_fd = open(dev, O_RDWR | O_NOCTTY)) < 0) {
    perror(__func__);
    return 1;
  }

  struct termios dev_io;
  tcgetattr(dev_fd, &dev_io);

  // Serial communication parameters
  // Completely raw communication, undefined special characters, no escape
  // characters, 8N1, no echo
  dev_io.c_iflag &= ~(IXON | IXOFF | IXANY | BRKINT | ICRNL | IMAXBEL);
  dev_io.c_iflag |= IGNBRK;
  dev_io.c_oflag = 0;
  dev_io.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
  dev_io.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | ISIG | ICANON | IEXTEN);
  dev_io.c_cflag |= CS8 | CREAD | CLOCAL; // 8N1

  // No default serial special characters
  memset(&dev_io.c_cc, '\0', sizeof(dev_io.c_cc));

  dev_io.c_cc[VMIN] = 1;
  dev_io.c_cc[VTIME] = 1;

  // Baud rate
  cfsetospeed(&dev_io, BAUD_RATE);
  cfsetispeed(&dev_io, BAUD_RATE);

  // Attrbutes are applied immediately
  tcsetattr(dev_fd, TCSANOW, &dev_io);
  tcflush(dev_fd, TCIFLUSH);

  return 0;
}


// Close the currently open serial device
// Returns 0 on success, 1 on failure
int serial_close(void) {
  if (!valid_dev()) return 1;
  int old_fd = dev_fd;
  dev_fd = -1;
  return close(old_fd);
}


// Receive data from the serial port
// Returns the number of read bytes, or -1 on failure
int serial_rx(void *dest, unsigned size) {
  if (!valid_dev() || !dest) return 0;
  unsigned bytes_read = 0;

  while (bytes_read < size) {
    ssize_t r = read(dev_fd, dest + bytes_read, size - bytes_read);
    if (r < 0) return -1;
    else bytes_read += r;
  }

  return size;
}


// Send data to the serial port
// Returns the number of written bytes, or -1 on failure
int serial_tx(const void *buf, unsigned size) {
  if (!valid_dev()) return -1;
  unsigned written = 0;

  while (written < size) {
    ssize_t r = write(dev_fd, buf + written, size - written);
    if (r < 0) return -1;
    else written += r;
  }

  tcdrain(dev_fd);
  return size;
}
