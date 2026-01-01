#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "shared_protocol.h"

#include <openssl/crypto.h>
#include <stdint.h>

void update_lookup_server(const char *, ip_addr_t, uint16_t, SSL_CTX *);

void receive_messages();

#endif

