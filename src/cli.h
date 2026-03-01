#ifndef CHAT_CLI_H
#define CHAT_CLI_H

#include "database.h"
#include "server.h"
#include <stdbool.h>
#include <sqlite3.h>
#include <openssl/ssl.h>

void terminate(int);


void clear_screen();

void header_print(const char *);

void display_chat_interface(sqlite3 *, int, const char *, const char *, SSL_CTX *);

void handle_choice(int);

void cli_loop(sqlite3 *, const char *, SSL_CTX *);

#endif
