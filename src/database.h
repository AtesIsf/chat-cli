#ifndef CHAT_DB_H
#define CHAT_DB_H

#include <sqlite3.h>

#define DB_DIR ("/.chat-cli")
#define DB_NAME ("/data.db")

sqlite3 *initialize_db();

const char **get_chats(sqlite3 *, int *);

#endif
