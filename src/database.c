#include "database.h"

#include <assert.h>
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

  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Could not open the database! Exiting...\n");
  }

  const char *sql_chats = "CREATE TABLE IF NOT EXISTS chats(" \
                          "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                          "username TEXT UNIQUE NOT NULL);";
  const char *sql_msgs = "CREATE TABLE IF NOT EXISTS messages(" \
                         "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                         "chat_id INTEGER NOT NULL," \
                         "is_sent INTEGER NOT NULL," \
                         "content TEXT NOT NULL," \
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP," \
                         "FOREIGN KEY (chat_id) REFERENCES chats(id) ON DELETE CASCADE);";
  
  char *error_msg = NULL;
  status_code = sqlite3_exec(db, sql_chats, NULL, NULL, &error_msg);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Error creating table \"chats\": %s\n", error_msg);
    sqlite3_free(error_msg);
  }

  status_code = sqlite3_exec(db, sql_msgs, NULL, NULL, &error_msg);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Error creating table \"messages\": %s\n", error_msg);
    sqlite3_free(error_msg);
  }

  return db;
}

/*
 * Reads and returns the name of all the chats from the database.
 * Returns NULL if no chats are found. Updates the given counter
 * with the number of found chats. Asserts that none of the
 * parameters are NULL. The returned string array must be freed!
 */

const char **get_chats(sqlite3 *db, int *n_chats) {
  assert(db != NULL && n_chats != NULL);
  *n_chats = 0;

  sqlite3_stmt *statement = NULL;
  int status_code = 0;

  const char *sql_counter = "SELECT COUNT(*) FROM chats;";
  status_code = sqlite3_prepare_v2(db, sql_counter, -1, &statement, NULL);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Could not count the number of chats.");
    return NULL;
  }
  if (sqlite3_step(statement) == SQLITE_ROW) {
    *n_chats = sqlite3_column_int(statement, 0);
  }
  sqlite3_finalize(statement);
  statement = NULL;
  if (*n_chats == 0) {
    return NULL;
  }

  const char **usernames = malloc(sizeof(char *) * (*n_chats));
  assert(usernames != NULL);

  const char *sql_usernames = "SELECT username FROM chats;";
  status_code = sqlite3_prepare_v2(db, sql_usernames, -1, &statement, NULL);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Could not read usernames from chats.");
    free(usernames);
    usernames = NULL;
    return usernames;
  }

  int index = 0;
  while(sqlite3_step(statement) == SQLITE_ROW) {
    const char *username = (const char *) sqlite3_column_text(statement, 0);
    if (username != NULL) {
      usernames[index] = strdup(username);
    } else {
      usernames[index] = NULL;
    }
    index++;
  }
  sqlite3_finalize(statement);
  statement = NULL;

  return usernames;
}

