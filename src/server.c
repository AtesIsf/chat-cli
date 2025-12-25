#include "server.h"

#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <openssl/crypto.h>
#include <openssl/core.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/provider.h>
#include <netinet/in.h>

/*
 * Sends a message to the given address. Asserts that
 * the parameters are not NULL
 */

void send_message(comms_t *comms, const char *addr) {
  assert(comms != NULL && addr != NULL);
}

void deinitialize_server(comms_t *comms) {
  assert(comms != NULL);
}

