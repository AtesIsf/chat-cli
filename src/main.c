#include "cli.h"
#include "database.h"
#include "rsa.h"

#include <sqlite3.h>
#include <stdlib.h>

int main() {
  sieve_primes();
  if (!global_primes_calculated) {
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
  free(global_primes);
  global_primes = NULL;
  return 0;
}
