// Smart House
// Client main routine
#include <stdio.h>
#include <stdlib.h>
#include "serial.h"
#include "shell.h"

#define print_usage_and_exit(status) do {\
  fprintf(stderr, "Usage: %s [path-to-avr-device]\n", argv[0]);\
  exit(status);\
} while (0)

int main(int argc, const char *argv[]) {
  if (argc > 2) print_usage_and_exit(EXIT_FAILURE);

  const char *dev_path;
  if (argc == 1) {
    fprintf(stderr, "Warning: no device path given, assuming /dev/ttyUSB0");
    dev_path = "/dev/ttyUSB0";
  }
  else dev_path = argv[1];

  serial_open(dev_path);
  config_fetch();

  shell_main();

  serial_close();
  exit(EXIT_SUCCESS);
}
