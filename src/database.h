#ifndef CHAT_DB_H
#define CHAT_DB_H

#include <stdbool.h>
#include <sqlite3.h>
#include <stddef.h>

#define DEFAULT_SERVER_PORT (50505)
#define DEFAULT_CLIENT_PORT (50506)

#define DATA_DIR ("/.chat-cli")
#define DB_NAME ("/data.db")

typedef struct Message {
  char *content;
  char *timestamp;
  bool is_sent;
} msg_t;

sqlite3 *initialize_db();

const char **get_chats(sqlite3 *, int *);

int get_id_of_username(sqlite3 *, const char *);

msg_t *get_messages_from_chat_id(sqlite3 *, int, int *);

#endif
