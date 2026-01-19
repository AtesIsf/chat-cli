#include "server.h"

#include "database.h"
#include "shared_protocol.h"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

bool global_terminate_program = false;

void handle_terminate(int n) {
  global_terminate_program = true;
}

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
  sprintf(message, "U|%s|", username);

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
  char response_buf[64] = { '\0' };
  int bytes_read = SSL_read(ssl, response_buf, 63);
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

/*
 * Fetches and returns the requested user's ip address from the lookup
 * server. Throws an assertion error if any of the parameters are NULL.
 * Sets the reference-passed boolean to false on failure.
 */

ip_addr_t fetch_user_ip(const char *username, ip_addr_t addr, SSL_CTX *ctx, bool *success) {
  assert(username != NULL && ctx != NULL && success != NULL);

  char message[36] = { '\0' };
  sprintf(message, "F|%s|", username);

  int fd = socket(addr.family, SOCK_STREAM, 0);

  if (fd < 0) {
    return (ip_addr_t) { 0 };
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
    *success = false;
    return (ip_addr_t) { 0 };
  }

  int status_code = connect(fd, (struct sockaddr *) &ss, ss_length);
  if (status_code != 0) {
    close(fd);
    *success = false;
    return (ip_addr_t) { 0 };
  }

  SSL *ssl = SSL_new(ctx);
  if (ssl == NULL) {
    close(fd);
    *success = false;
    return (ip_addr_t) { 0 };
  }

  SSL_set_fd(ssl, fd);

  if (SSL_connect(ssl) != 1) {
    SSL_free(ssl);
    close(fd);
    *success = false;
    return (ip_addr_t) { 0 };
  }

  SSL_write(ssl, message, strlen(message));
  char response_buf[64] = { '\0' };
  int bytes_read = SSL_read(ssl, response_buf, 63);
  if (bytes_read <= 0) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    *success = false;
    return (ip_addr_t) { 0 };
  }

  char ip_buf[64] = { '\0' };
  char ip_type = '\0';

  status_code = sscanf(response_buf, "%c|%63[^|]|", &ip_type, ip_buf);
  if (status_code != 2) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    *success = false;
    return (ip_addr_t) { 0 };
  }

  ip_addr_t result = { 0 };
  int convertion_result = 1;
  if (ip_type == '4') {
    result.family = AF_INET;
    convertion_result = inet_pton(AF_INET, ip_buf, &result.addr.v4);
  } else if (ip_type == '6') {
    result.family = AF_INET6;
    convertion_result = inet_pton(AF_INET6, ip_buf, &result.addr.v6);
  } else {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    *success = false;
    return (ip_addr_t) { 0 };
  }

  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(fd);

  if (convertion_result <= 0) {
    *success = false;
    return (ip_addr_t) { 0 };
  }
  *success = true;
  return result;
}

/*
 * Will run in the background, handles incoming messages.
 * Will throw an assertion if the passed argument is NULL.
 */

void receive_messages(SSL_CTX *ctx) {
  assert(ctx != NULL);

  int fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (fd < 0) {
    puts("[ERROR] Failed to create listening socket!");
    return;
  }

  int option = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  struct sockaddr_in6 addr = {
    .sin6_family = AF_INET6,
    .sin6_addr = in6addr_any,
    .sin6_port = htons(CLIENT_PORT),
  };

  int status_code = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
  if (status_code < 0) {
    puts("[ERROR] Could not bind to the client port!");
    close(fd);
    return;
  }

  status_code = listen(fd, 32);
  if (status_code < 16) {
    puts("[ERROR] An error occured while attempting to listen to incoming connections!");
    close(fd);
    return;
  }
  
  while (!global_terminate_program) {
    struct sockaddr_storage peer = { 0 };
    socklen_t peer_len = sizeof(peer);

    int client_fd = accept(fd, (struct sockaddr *) &peer, &peer_len);
    if (client_fd < 0) {
      puts("[WARNING] Could not accept incoming connection.");
      continue;
    }

    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL) {
      puts("[WARNING] Could not initialize SSL.");
      close(client_fd);
      continue;
    }

    SSL_set_fd(ssl, client_fd);

    if (SSL_accept(ssl) != 1) {
      puts("[WARNING] Could not SSL-Accept incoming connection.");
      SSL_free(ssl);
      close(client_fd);
      continue;
    }

    handle_incoming(ssl, &peer);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = NULL;
  }
  puts("[INFO] Closed client socket.");
}

/*
 * Handles incoming messages. Asserts given parameters are not NULL.
 * Throws an assertion on heap allocation failures.
 * Expects messages in the format "username|content"
 */

void handle_incoming(SSL *ssl, struct sockaddr_storage *peer) {
  assert(ssl != NULL && peer != NULL);
  char buf[256] = { '\0' };

  int n_bytes_read = SSL_read(ssl, buf, sizeof(buf) - 1);
  if (n_bytes_read <= 0) {
    return;
  }
  buf[n_bytes_read] = '\0';

  // Here, I dont have to validate username length as the program
  // does it at the start and even if it somehow didn't, it doesn't break anything
  char username[32] = { '\0' };
  int status_code = sscanf(buf, "%31[^|]|", username);
  username[31] = '\0';
  if (status_code != 1) {
    SSL_write(ssl, ERR_RESPONSE, strlen(ERR_RESPONSE));
    return;
  }

  // TODO: Username pubkey validation here

  int buf_len = strlen(buf);
  int index = 0;
  while (buf[index] != '|' && index < buf_len) index++;
  char *content_start = &buf[++index];
  if (index == buf_len) {
    SSL_write(ssl, ERR_RESPONSE, strlen(ERR_RESPONSE));
    return;
  }

  time_t now = time(NULL);
  struct tm time_now = { 0 };
  char time_buf[32] = { '\0' };

  size_t n = strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &time_now);

  msg_t msg = {
    .is_sent = false,
    .content = content_start,
    .timestamp = time_buf,
  };

  // TODO: Write msg to DB

  // OK
  SSL_write(ssl, OK_RESPONSE, strlen(OK_RESPONSE));
}

