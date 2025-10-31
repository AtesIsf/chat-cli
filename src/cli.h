#ifndef CHAT_CLI_H
#define CHAT_CLI_H

#include <stdbool.h>
#include <sqlite3.h>

volatile static bool terminate_program = false;

void terminate(int);

void clear_screen();

void header_print();

void handle_choice(int);

void cli_loop(sqlite3 *);

#endif
