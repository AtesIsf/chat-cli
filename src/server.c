#include "server.h"

#include <assert.h>
#include <openssl/crypto.h>

/*
 * Updates the lookup server with the current ip address
 * of the given username. Throws an assertion error if
 * the given username is NULL.
 */

void update_lookup_server(const char *username, ip_addr_t addr, uint16_t port, SSL_CTX *ctx) {
  assert(username != NULL);
}

