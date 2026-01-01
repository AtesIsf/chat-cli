#include "cli.h"
#include "database.h"
#include "server.h"
#include "shared_protocol.h"
#include "ssl.h"

#include <bits/sockaddr.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  char *username = NULL;
  if (argc != 2) {
    puts("[ERROR] Incorrect format! Please run the program as such: \"chat-cli <USERNAME>\"");
    puts("Exiting...");
    return 1;
  }
  if (strlen(argv[1]) >= 32) {
    puts("[ERROR] The given username must be shorter than 30 characters!");
    puts("Exiting...");
    return 1;
  }
  username = argv[1];
  username[31] = '\0';

  get_cert_dirs();
  SSL_CTX *ctx = init_openssl();
  if (ctx == NULL) {
    return 1;
  }
  sqlite3 *db = initialize_db();
  if (db == NULL) {
    return 1;
  }

  // TODO: MAKE THIS ADJUSTABLE LATER!!! Now, its localhost for development purposes
  ip_addr_t temp = (ip_addr_t) { .family = AF_INET, .addr.v4.s_addr = htonl(INADDR_LOOPBACK)};
  update_lookup_server(username, temp, LOOKUP_PORT, ctx);
  cli_loop(db, username);

  sqlite3_close(db);
  SSL_CTX_free(ctx);
  ctx = NULL;
  return 0;
}
