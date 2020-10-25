// Smart House - Nicola Colao
// Client Shell
#ifndef __CLIENT_SHELL_MODULE_H
#define __CLIENT_SHELL_MODULE_H

#define SH_OUT_STREAM stdout
#define SH_IN_STREAM stdin
#define SH_PROMPT "> "
#define SH_LINE_MAX_LEN 510

// Shell command data type
typedef struct _shell_cmd_s {
  char *name;
  void (*execute)(int argc, char *argv[]);
} shell_cmd_t;

// Initialize and launch the client shell
void shell_main(void);

#endif
