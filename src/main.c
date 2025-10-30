#include "cli.h"
#include "database.h"

#include <sqlite3.h>
#include <stdio.h>

int main() {
  sqlite3 *db = initialize_db();
  if (db == NULL) {
    return 1;
  }

  cli_loop();

  sqlite3_close(db);
  return 0;
}
