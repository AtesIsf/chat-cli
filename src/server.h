#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "shared_protocol.h"

#include <openssl/crypto.h>
#include <stdbool.h>

int update_lookup_server(const char *, ip_addr_t, SSL_CTX *);

ip_addr_t fetch_user_ip(const char *, SSL_CTX *, bool *);

void receive_messages();

#endif

