#include "cli.h"
#include "database.h"
#include "server.h"

#include <sqlite3.h>
#include <stdlib.h>

int main() {
  configs_t *conf = read_configs();
  if (conf == NULL) {
    return 1;
  }
  sqlite3 *db = initialize_db();
  if (db == NULL) {
    return 1;
  }

  cli_loop(db);

  sqlite3_close(db);
  free(conf);
  conf = NULL;
  return 0;
}
