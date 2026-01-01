#include "server.h"

#include <assert.h>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Updates the lookup server with the current ip address
 * of the given username. Throws an assertion error if
 * the given parameters are NULL or if the username is not less than 32.
 * Returns 0 on success and -1 on failure.
 */

int update_lookup_server(const char *username, ip_addr_t addr, SSL_CTX *ctx) {
  assert(username != NULL && ctx != NULL);
  assert(strlen(username) < 32);

  char message[36] = { '\0' };
  sprintf(message, "U|%s", username);

  int fd = socket(addr.family, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  struct sockaddr_storage ss = { 0 };
  socklen_t ss_length = 0;

  if (addr.family == AF_INET) {
    struct sockaddr_in *sin = (struct sockaddr_in *) &ss;
    sin->sin_family = addr.family;
    sin->sin_addr = addr.addr.v4;
    sin->sin_port = htons(LOOKUP_PORT);
    ss_length = sizeof(*sin);
  } else if (addr.family == AF_INET6) {
    struct sockaddr_in6 *sin = (struct sockaddr_in6 *) &ss;
    sin->sin6_family = addr.family;
    sin->sin6_addr = addr.addr.v6;
    sin->sin6_port = htons(LOOKUP_PORT);
    ss_length = sizeof(*sin);
  } else {
    close(fd);
    return -1;
  }

  int status_code = connect(fd, (struct sockaddr *) &ss, ss_length);
  if (status_code != 0) {
    close(fd);
    return -1;
  }

  SSL *ssl = SSL_new(ctx);
  if (ssl == NULL) {
    close(fd);
    return -1;
  }

  SSL_set_fd(ssl, fd);

  if (SSL_connect(ssl) != 1) {
    SSL_free(ssl);
    close(fd);
    return -1;
  }

  SSL_write(ssl, message, strlen(message));
  char response_buf[16] = { '\0' };
  int bytes_read = SSL_read(ssl, response_buf, 15);
  if (bytes_read <= 0) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    return -1;   
  }

  if (strncmp(response_buf, OK_RESPONSE, strlen(OK_RESPONSE)) != 0) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    return -1;   
  }

  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(fd);
  return 0;
}

