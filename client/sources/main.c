// Smart House
// Client main routine
#include <stdio.h>
#include <stdlib.h>

#include "communication.h"

#include "serial.h"
#include "shell.h"
#include "config.h"

#define print_usage_and_exit(status) do {\
  fprintf(stderr, "Usage: %s [path-to-avr-device]\n", argv[0]);\
  exit(status);\
} while (0)


int main(int argc, const char *argv[]) {
  if (argc > 2) print_usage_and_exit(EXIT_FAILURE);

  const char *dev_path;
  if (argc == 1) {
    fprintf(stderr, "Warning: no device path given, assuming /dev/ttyUSB0\n");
    dev_path = "/dev/ttyUSB0";
  }
  else dev_path = argv[1];

  if (serial_open(dev_path)) {
    fprintf(stderr, "Error: Could not open device %s\n", dev_path);
    exit(EXIT_FAILURE);
  }

  if (config_fetch()) {
    fprintf(stderr, "Error: Could not fetch AVR board configuration\n");
    exit(EXIT_FAILURE);
  }

  shell_main();

  if (serial_close()) {
    fprintf(stderr, "Warning: Error while closing %s\n", dev_path);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
