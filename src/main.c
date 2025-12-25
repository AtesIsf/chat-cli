#include "cli.h"
#include "database.h"
#include "server.h"

#include <openssl/ssl.h>
#include <sqlite3.h>
#include <stdlib.h>

int main() {
  get_cert_dirs();
  SSL_CTX *ctx = init_openssl();
  if (ctx == NULL) {
    return 1;
  }
  configs_t *conf = read_configs();
  if (conf == NULL) {
    return 1;
  }
  sqlite3 *db = initialize_db();
  if (db == NULL) {
    return 1;
  }

  cli_loop(db, conf);

  sqlite3_close(db);
  free(conf);
  conf = NULL;
  SSL_CTX_free(ctx);
  ctx = NULL;
  return 0;
}
