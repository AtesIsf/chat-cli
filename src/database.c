#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <openssl/sha.h>

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

  size_t dir_len = strlen(DATA_DIR);
  memcpy(database_str + len, DATA_DIR, dir_len);
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
                          "username TEXT UNIQUE NOT NULL," \
                          "fingerprint BLOB NOT NULL);";
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

/*
 * Fetches the corresponding id of the given username from the databse.
 * Asserts the parameters are not NULL. Returns -1 if the given username
 * does not exist in the database.
 */

int get_id_of_username(sqlite3 *db, const char *username) {
  assert(db != NULL && username != NULL);
  int id = -1;

  const char *cmd = "SELECT id FROM chats WHERE username = ?;";
  sqlite3_stmt *statement = NULL;

  int status_code = sqlite3_prepare_v2(db, cmd, -1, &statement, NULL);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] A problem occured while preparing to fetch id.\n");
    return id;
  }

  sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
  status_code = sqlite3_step(statement);

  if (status_code == SQLITE_ROW) {
    id = sqlite3_column_int(statement, 0);
  } else {
    fprintf(stderr, "[ERROR] A problem occured while fetching id.\n");
  }

  sqlite3_finalize(statement);
  return id;
}

/*
 * Fetches and returns a heap-allocated string that stores the
 * fingerprint of the requested user. Returns NULL if the
 * fingerprint cannot be found. Throws an assertion if any of
 * the parameters are NULL or if a heap allocation error occurs.
 * The returned pointer must be freed afterwards!!! The returned
 * string has size SHA256_DIGEST_LENGTH + 1 as a null
 * terminator is added to the end.
 */

unsigned char *get_fingerprint(sqlite3 *db, const char *username) {
  assert(username != NULL);

  const char *cmd = "SELECT fingerprint FROM chats WHERE username = ?;";

  sqlite3_stmt *statement = NULL;
  int status_code = sqlite3_prepare_v2(db, cmd, -1, &statement, NULL);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Failed to prepare to fetch requested fingerprint\n");
    return NULL;
  }

  sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
  if (sqlite3_step(statement) != SQLITE_ROW) {
    sqlite3_finalize(statement);
    return NULL;
  }
  const void *blob = sqlite3_column_blob(statement, 0);
  int n_bytes = sqlite3_column_bytes(statement, 0);
  if (blob == NULL || n_bytes != SHA256_DIGEST_LENGTH) {
    sqlite3_finalize(statement);
    return NULL;
  }

  unsigned char *fingerprint = calloc(sizeof(unsigned char), SHA256_DIGEST_LENGTH + 1);
  assert(fingerprint != NULL);
  fingerprint[SHA256_DIGEST_LENGTH] = '\0';
  memcpy(fingerprint, blob, SHA256_DIGEST_LENGTH);

  sqlite3_finalize(statement);
  return fingerprint;
}

/*
 * Gets all messages of the chat with the given id in a sorted manner. Updates
 * the given int * to match the number of messages found. Asserts the given
 * pointers are not NULL and the given chat id is nonnegative. Also throws an
 * assertion error if a malloc fails.
 */

msg_t *get_messages_from_chat_id(sqlite3 *db, int chat_id, int *n_msgs) {
  assert(db != NULL && chat_id >= 0 && n_msgs != NULL);
  msg_t *messages = NULL;
  *n_msgs = 0;

  const char *count_cmd = "SELECT COUNT(*) FROM messages WHERE chat_id = ?;";
  sqlite3_stmt *count_statement = NULL;

  int status_code = sqlite3_prepare_v2(db, count_cmd, -1, &count_statement, NULL);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Failed to prepare to count messages.");
    return messages;
  }
  sqlite3_bind_int(count_statement, 1, chat_id);
  status_code = sqlite3_step(count_statement);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Failed to count messages.");
    sqlite3_finalize(count_statement);
    return messages;
  }
  *n_msgs = sqlite3_column_int(count_statement, 0);
  sqlite3_finalize(count_statement);

  messages = malloc(sizeof(msg_t) * (*n_msgs));
  assert(messages != NULL);

  const char *cmd = "SELECT is_sent, content, timestamp FROM messages WHERE chat_id = ? ORDER BY timestamp ASC;";

  sqlite3_stmt *statement = NULL;
  status_code = sqlite3_prepare_v2(db, cmd, -1, &statement, NULL);
  if (status_code != SQLITE_OK) {
    fprintf(stderr, "[ERROR] Failed to prepare to fetch messages\n");
    return messages;
  }

  sqlite3_bind_int(statement, 1, chat_id);

  int index = 0;
  while (sqlite3_step(statement) == SQLITE_ROW) {
    int is_sent = sqlite3_column_int(statement, 0);
    const char *content = (const char *) sqlite3_column_text(statement, 1);
    const char *timestamp = (const char *) sqlite3_column_text(statement, 2);

    if (content == NULL) {
      content = "";
    }
    if (timestamp == NULL) {
      timestamp = "";
    }

    messages[index].is_sent = is_sent == 1 ? true : false; 
    messages[index].content = strdup(content);
    messages[index].timestamp = strdup(timestamp);
    
    index++;
  }

  sqlite3_finalize(statement);
  return messages;
}

