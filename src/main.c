#include "cli.h"
#include "database.h"
#include "server.h"
#include "shared_protocol.h"
#include "ssl.h"

#include <assert.h>
#include <bits/sockaddr.h>
#include <signal.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool is_valid_username(const char *username) {
  assert(username != NULL);
  for (size_t i = 0; i < strlen(username); i++) {
    if ((username[i] >= 'a' && username[i] <= 'z') || (username[i] >= '0' && username[i] <= '9')) {
      continue;
    } else {
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  signal(SIGINT, handle_terminate);

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
  if (!is_valid_username(argv[1])) {
    puts("[ERROR] The username can only contain numbers and lowercase characters!");
    return 1;
  }

  char username[32] = { '\0' };
  strncpy(username, argv[1], 32);
  username[31] = '\0';

  get_cert_dirs();

  SSL_CTX *client_ctx = init_openssl(CLIENT);
  if (client_ctx == NULL) {
    return 1;
  }
  SSL_CTX *server_ctx = init_openssl(SERVER);
  if (server_ctx == NULL) {
    return 1;
  }

  sqlite3 *db = initialize_db();
  if (db == NULL) {
    return 1;
  }


  // TODO: MAKE THIS ADJUSTABLE LATER!!! Now, its localhost for development purposes
  ip_addr_t temp = (ip_addr_t) { .family = AF_INET, .addr.v4.s_addr = htonl(INADDR_LOOPBACK)};
  int status_code = update_lookup_server(username, temp, client_ctx);
  if (status_code == -1) {
    puts("[WARNING] Could not update the lookup server with the current IP.");
  }
  cli_loop(db, username);

  sqlite3_close(db);
  SSL_CTX_free(client_ctx);
  SSL_CTX_free(server_ctx);
  client_ctx = NULL;
  server_ctx = NULL;
  return 0;
}
