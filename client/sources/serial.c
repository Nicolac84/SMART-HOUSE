// Smart House
// Serial module
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

#include "serial.h"

static int dev_fd = -1;
 
static inline int valid_dev(void) {
  return dev_fd > 2 && isatty(dev_fd) ? 1 : 0;
}


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
  dev_io.c_oflag = 0;
  dev_io.c_lflag &= ~(ECHO | ECHOE | ISIG | ICANON);
  dev_io.c_iflag &= ~(IXON | IXOFF | IXANY);
  dev_io.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
  dev_io.c_cc[VMIN] = 1;
  dev_io.c_cc[VTIME] = 30;
  dev_io.c_cflag |= CS8 | CREAD | CLOCAL; // 8N1


  // Baud rate
  cfsetospeed(&dev_io, BAUD_RATE);
  cfsetispeed(&dev_io, BAUD_RATE);

  // Attrbutes are applied immediately
  tcsetattr(dev_fd, TCSANOW, &dev_io);
  tcflush(dev_fd, TCIFLUSH);

  return 0;
}


int serial_close(void) {
  if (!valid_dev()) return 1;
  int old_fd = dev_fd;
  dev_fd = -1;
  return close(old_fd);
}


int serial_rx(void *dest, unsigned size) {
  return (!valid_dev() || !dest) ? 0 : read(dev_fd, dest, size);
}


int serial_tx(const void *buf, unsigned size) {
  if (!valid_dev()) return -1;
  int written = write(dev_fd, buf, size);
  if (written < 0) return -1;
  tcdrain(dev_fd);
  return written;
}
