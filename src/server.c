#include "server.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

/*
 * Initializes and returns a struct that stores the
 * server and client sockets of the application. The
 * returned struct pointer must be deinitialized using
 * deinitialize_server() afterwards to prevent memory leaks.
 * Throws an assertion error if heap memory allocation fails.
 */

comms_t *initialize_server() {
  comms_t *comms = malloc(sizeof(comms_t));
  assert(comms != NULL);


  return comms;
}

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

