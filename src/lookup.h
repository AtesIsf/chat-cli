#ifndef CHAT_LOOKUP_H
#define CHAT_LOOKUP_H

#include "shared_protocol.h"

#include <netinet/in.h>
#include <openssl/crypto.h>
#include <stdbool.h>

#define INITIAL_TABLE_SIZE (16)
#define MAX_TABLE_SIZE (1048576) // 2 ^ 20
#define HASH_PRIME (7)
#define LOAD_FACTOR (0.67)
#define RESIZE_FACTOR (2)
#define MAX_USERNAME_LEN (32)
#define PORT (56732)

#define STORAGE_FILE ("/table.txt")

extern char global_table_filename[256];

extern volatile bool global_terminate_program;

typedef struct UserData {
  char username[32];
  ip_addr_t ip;
  bool tombstone;
} userdata_t;

typedef struct HashTable {
  userdata_t *map;
  size_t size;
  size_t n_elements;
} hashtable_t;

void terminate_signal(int);

void print_table(const hashtable_t *);

void generate_table_filename();

void write_table(hashtable_t *);

hashtable_t generate_hashmap();

void free_hashmap(hashtable_t *);

int get_index(hashtable_t *, const char *);

void resize(hashtable_t *);

int insert(hashtable_t *, userdata_t);

int delete_data(hashtable_t *, const char *);

char *handle_fetch(const char *, hashtable_t *);

char *handle_update(const char *, hashtable_t *);

void endpoint_manager(SSL_CTX *, hashtable_t *);

int main();

#endif
