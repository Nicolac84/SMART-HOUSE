// Smart House
// Client shell module
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "shell.h"
#include "config.h"
#include "communication.h"
#include <ctype.h>
#define PIN_NAME_MAX_LEN 64

// Print an error message and exit from a shell command
#define sh_error(msg) do {\
  fprintf(SH_OUT_STREAM, "%s\n", msg);\
  return;\
} while (0)


// Get the pin number from its identifying string (e.g. PWM0)
static com_pin_t get_pin_from_id(const char *name) {
  // Uppercase local copy for 'name'
  char *_name = strndup(name, PIN_NAME_MAX_LEN);
  if (!_name) return COM_PIN_NULL; // TODO: Abort the entire program?
  for (char *s = _name; *s; ++s) *s = toupper(*s);

  unsigned char pin;
  com_pin_t ret = COM_PIN_NULL;
  if (sscanf(_name, "PWM%hhu", &pin))
    ret = (pin <= 7) ? pin + COM_PWM0 : COM_PIN_NULL;
  else if (sscanf(_name, "ANALOG%hhu", &pin))
    ret = (pin <= 7) ? pin + COM_ANALOG0 : COM_PIN_NULL;
  else if (sscanf(_name, "DIGITIN%hhu", &pin))
    ret = (pin <= 7) ? pin + COM_DIGITIN0 : COM_PIN_NULL;

  free(_name);
  return ret;
}


/* --------------------------
 *  Shell commands
 * -------------------------- */
static shell_cmd_t cmd_table[];
static const size_t cmd_count;

// Print a list of all the commands
static void cmd_help(int argc, char *argv[]) {
  for (size_t i=0; i < cmd_count; ++i)
    fprintf(SH_OUT_STREAM, "%s\n", cmd_table[i].name);
}

// Simple echo (mostly for shell testing)
static void cmd_echo(int argc, char *argv[]) {
  for (size_t i=1; i < argc-1; ++i)
    fprintf(SH_OUT_STREAM, "%s ", argv[i]);
  fprintf(SH_OUT_STREAM, "%s\n", argv[argc-1]);
}


// List all pins (along with names)
static void cmd_list(int argc, char *argv[]) {
  if (argc != 1) sh_error("Usage: list");
  fputs("\nPIN      | TYPE    | PIN NAME\n"
        "---------+---------+---------------------------------------\n",
        SH_OUT_STREAM);


  for (unsigned char pin=COM_PWM0; pin <= COM_PWM7; ++pin)
    fprintf(SH_OUT_STREAM, "PWM%1hhu     | PWM     | %s\n",
        pin - COM_PWM0, config_get_pin_name(pin));

  for (unsigned char pin=COM_ANALOG0; pin <= COM_ANALOG7; ++pin)
    fprintf(SH_OUT_STREAM, "ANALOG%1hhu  | ANALOG  | %s\n",
        pin - COM_ANALOG0, config_get_pin_name(pin));

  for (unsigned char pin=COM_DIGITIN0; pin <= COM_DIGITIN7; ++pin)
    fprintf(SH_OUT_STREAM, "DIGITIN%1hhu | DIGITIN | %s\n",
        pin - COM_DIGITIN0, config_get_pin_name(pin));

  fputc('\n', SH_OUT_STREAM);
}


// Get name for a pin
static void cmd_get_pin_name(int argc, char *argv[]) {
  if (argc != 2) sh_error("Usage: get_pin_name <pin>\n"
      "Possible values are PWM[1-7], ANALOG[1-7] and DIGITIN[1,7]");

  com_pin_t pin = get_pin_from_id(argv[1]);
  if (pin == COM_PIN_NULL)
    sh_error("Could not get given pin");

  const char *name = config_get_pin_name(pin);
  fprintf(SH_OUT_STREAM, "%s\n", name ? name : "<no name>");
}


// Set name for a pin
static void cmd_set_pin_name(int argc, char *argv[]) {
  if (argc != 3) sh_error("Usage: set_pin_name <pin> <name>\n"
      "Possible pin values are PWM[1-7], ANALOG[1-7] and DIGITIN[1,7]");

  com_pin_t pin = get_pin_from_id(argv[1]);
  if (pin == COM_PIN_NULL)
    sh_error("Could not get given pin");

  communication_set_pin_name(pin, argv[2]);
  config_set_pin(pin, argv[2]);
}


// Get current value for a pin
static void cmd_get_pin_value(int argc, char *argv[]) {
  if (argc != 2) sh_error("Usage: get_pin_value <pin-name>");

  com_pin_t pin = config_get_pin(argv[1]);
  if (pin == COM_PIN_NULL)
    sh_error("Could not get given pin");

  com_handle_send_void(pin, COM_OP_GET_VAL, 0, 0, SH_OUT_STREAM,
      "Error while sending packet");

  packet_t p[1];
  com_handle_recv_void(p, SH_OUT_STREAM, "Error while receiving packet");

  fprintf(SH_OUT_STREAM, "value = %hhu \n", *((uint8_t*) p->body));
}


// Set current value for a pin
static void cmd_set_pin_value(int argc, char *argv[]) {
  if (argc != 3) sh_error("Usage: set_pin_value <pin> <pin-name>");

  com_pin_t pin = config_get_pin(argv[1]);
  if (pin == COM_PIN_NULL)
    sh_error("Could not get given pin");
  uint8_t val =(uint8_t) atoi(argv[2]);
  com_handle_send_void(pin, COM_OP_SET_VAL, 1, &val, SH_OUT_STREAM,
      "Error while sending packet");

  packet_t p[1];
  com_handle_recv_void(p, SH_OUT_STREAM, "Error while receiving packet");
}


// Turn a pin ON
static void cmd_pin_on(int argc, char *argv[]) {
  if (argc != 2) sh_error("Usage: pin_on <pin-name>");

  com_pin_t pin = config_get_pin(argv[1]);
  if (pin == COM_PIN_NULL)
    sh_error("Could not get given pin");

  com_handle_send_void(pin, COM_OP_ON, 0, NULL, SH_OUT_STREAM,
      "Error while sending packet");

  packet_t p[1];
  com_handle_recv_void(p, SH_OUT_STREAM, "Error while receiving packet");
}

// Turn a pin OFF
static void cmd_pin_off(int argc, char *argv[]) {
  if (argc != 2) sh_error("Usage: pin_off <pin-name>");

  com_pin_t pin = config_get_pin(argv[1]);
  if (pin == COM_PIN_NULL)
    sh_error("Could not get given pin");

  com_handle_send_void(pin, COM_OP_OFF, 0, NULL, SH_OUT_STREAM,
      "Error while sending packet");

  packet_t p[1];
  com_handle_recv_void(p, SH_OUT_STREAM, "Error while receiving packet");
}

// Make the AVR board save its current status into its EEPROM memory
static void cmd_save_status(int argc, char *argv[]) {
  if (argc != 1) sh_error("Usage: save-status");

  com_handle_send_void(COM_PIN_NULL, COM_OP_SAVE_STATUS, 0, NULL,
      SH_OUT_STREAM, "Error while sending packet");

  packet_t p[1];
  com_handle_recv_void(p, SH_OUT_STREAM, "Error while receiving packet");
}


// Commands table
static shell_cmd_t cmd_table[] = {
  { .name = "help", .execute = cmd_help },
  { .name = "echo", .execute = cmd_echo },
  { .name = "list", .execute = cmd_list },
  { .name = "get-pin-name", .execute = cmd_get_pin_name },
  { .name = "set-pin-name", .execute = cmd_set_pin_name },
  { .name = "get-pin-value", .execute = cmd_get_pin_value },
  { .name = "set-pin-value", .execute = cmd_set_pin_value },
  { .name = "pin-on", .execute = cmd_pin_on },
  { .name = "pin-off", .execute = cmd_pin_off },
  { .name = "save-status", .execute = cmd_save_status }
};

static const size_t cmd_count = sizeof(cmd_table) / sizeof(shell_cmd_t);


/* --------------------------
 *  Module functions
 * -------------------------- */

// Find a command (returns NULL if not found)
static shell_cmd_t *cmd_find(const shell_cmd_t *table, const char *name) {
  if (!table || !name) return NULL;
  for (size_t i=0; i < cmd_count; ++i)
    if (strcmp(cmd_table[i].name, name) == 0) return cmd_table + i;
  return NULL;
}

// Tokenize a string
static int tokenize(char *line, char *argv[]) {
  int argc;
  argv[0] = strtok(line, " \n");
  if (!argv[0]) return 0;
  for (argc = 1; (argv[argc] = strtok(NULL, " \n")) != NULL; ++argc)
    ;
  return argc;
}

// Initialize and launch the client shell
void shell_main(void) {
  fputs(
      "Smart House client\n"
      "Run 'help' for a list of available commands\n"
      "Press CTRL+D to exit\n"
      "\n", SH_OUT_STREAM);

  // User input line
  char line[SH_LINE_MAX_LEN + 2];

  // argc-argv variables for commands execution
  int argc;
  char *argv[SH_LINE_MAX_LEN / 2 + 1];

  while (1) {
    fputs(SH_PROMPT, SH_OUT_STREAM); // Print shell prompt

    // Get user input line
    if (!fgets(line, SH_LINE_MAX_LEN + 2, SH_IN_STREAM)) { // No line was read
      if (feof(SH_IN_STREAM))
        break;
      else if (ferror(SH_IN_STREAM) && errno != EINTR)
        perror(__func__);
      continue;
    }

    argc = tokenize(line, argv); // Tokenize the string
    if (!argc) continue; // Skip blank lines

    // Look for the command (first token)
    const shell_cmd_t *cmd = cmd_find(cmd_table, argv[0]);

    if (cmd)
      cmd->execute(argc, argv);
    else
      fputs("Command not found\n", SH_OUT_STREAM);
  }

  fputc('\n', SH_OUT_STREAM);
}
