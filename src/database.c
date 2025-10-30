#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sqlite3.h>

/*
 * Intitializes a local sqlite3 database. Opens it if one already exists,
 * creates a new one if it doesn't. Returns NULL on an error. Database
 * exists at ~/.chat-cli
 */

sqlite3 *initialize_db() {
  const char *home_dir = getenv("HOME");
  char database_str[256] = { '\0' };
  size_t len = strlen(home_dir);

  memcpy(database_str, home_dir, len);
  database_str[255] = '\0';

  size_t dir_len = strlen(DB_DIR);
  memcpy(database_str + len, DB_DIR, dir_len);
  database_str[255] = '\0';

  mkdir(database_str, 0755);

  memcpy(database_str + len + dir_len, DB_NAME, strlen(DB_NAME));
  database_str[255] = '\0';

  sqlite3 *db = NULL;

  int status_code = sqlite3_open(database_str, &db);

  if (status_code != 0) {
    fprintf(stderr, "[ERROR] Could not open the database! Exiting...\n");
  }

  return db;
}
