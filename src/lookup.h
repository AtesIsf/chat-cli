#ifndef CHAT_LOOKUP_H
#define CHAT_LOOKUP_H

#include "rsa.h"

#define INITIAL_TABLE_SIZE (32)
#define MAX_TABLE_SIZE (1048576) // 2 ^ 20
#define HASH_PRIME (7)
#define LOAD_FACTOR (0.67)
#define RESIZE_FACTOR (2)

typedef struct UserData {
  rsa_t keys;
  char *username;
  char *ip; // May change later
  bool tombstone;
} userdata_t;

typedef struct HashTable {
  userdata_t *map;
  size_t size;
  size_t n_elements;
} hashtable_t;

hashtable_t generate_hashmap();

void free_hashmap(hashtable_t *);

int get_index(hashtable_t *, const char *);

void resize(hashtable_t *);

int insert(hashtable_t *, userdata_t);

int delete_data(hashtable_t *, const char *);

int main();

#endif
