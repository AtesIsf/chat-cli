#ifndef CHAT_CLI_H
#define CHAT_CLI_H

#include "database.h"
#include <stdbool.h>
#include <sqlite3.h>

volatile static bool terminate_program = false;

void terminate(int);

void clear_screen();

void header_print();

void display_chat_interface(sqlite3 *, int, const char *);

void display_settings(configs_t *);

void handle_choice(int);

void cli_loop(sqlite3 *, configs_t *);

#endif
