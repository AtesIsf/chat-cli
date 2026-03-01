#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "shared_protocol.h"

#include <openssl/crypto.h>
#include <sqlite3.h>
#include <stdbool.h>

#define CLIENT_PORT (47906)
#define SERVER_PORT (47907)

typedef struct ServerArgs {
  SSL_CTX *ctx;
  sqlite3 *db;
} server_args_t;

extern bool global_terminate_program;

void handle_terminate(int);

int update_lookup_server(const char *, ip_addr_t, SSL_CTX *);

int send_message(const char *, const char *, ip_addr_t, SSL_CTX *);

ip_addr_t fetch_user_ip(const char *, ip_addr_t, SSL_CTX *, bool *);

void *receive_messages(void *);

void handle_incoming(SSL *, struct sockaddr_storage *, sqlite3 *);

#endif

