
/*
 * This file implements the minimal lookup server
 * required by this project. It enables users to
 * search for users they want to message.
 *
 * Implements a hash table and HTTP server.
 * Uses open addressing in the hash table.
 */

#include "rsa.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_TABLE_SIZE (32)
#define HASH_PRIME (7)
#define LOAD_FACTOR (0.67)

// TODO: reorder to minimize size later

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

/*
 * Generates the UserData hashmap. Throws an
 * assertion if a memory allocation error occurs.
 * Call free_hashmap() afterwards to avoid memory leaks!
 */

hashtable_t generate_hashmap() {
  userdata_t *map = calloc(INITIAL_TABLE_SIZE, sizeof(userdata_t));
  assert(map != NULL);
  hashtable_t ht = (hashtable_t){ .map = map, .size = INITIAL_TABLE_SIZE, .n_elements = 0 };
  return ht;
}

void free_hashmap(hashtable_t *hm) {
  userdata_t *map = hm->map;
  for (size_t i = 0; i < hm->size; i++) {
    if (map[i].username == NULL) {
      continue;
    }
    free(map[i].username);
    map[i].username = NULL;
    // TODO: change later if it changes in the struct
    free(map[i].ip);
    map[i].ip = NULL;
  }
  free(map);
  map = NULL;
}

/*
 * Finds and returns the index of the given username. Returns -1
 * if the given user does not exist in the hashtable. Throws an
 * assertion if any of the parameters are NULL.
 */

int get_index(hashtable_t *ht, const char *username) {
  assert(ht != NULL && username != NULL && ht->map != NULL);

  size_t name_len = strlen(username);
  int hash_val = 0;
  for (size_t i = 0; i < name_len; i++) {
    hash_val += ((int) username[i]) * pow(HASH_PRIME, i);
  }
  hash_val %= ht->size;

  // Case 1: does not exist
  if (ht->map[hash_val].tombstone == false && ht->map[hash_val].username == NULL) {
    return -1;
  }
  // Case 2: found
  else if(ht->map[hash_val].tombstone == false && strcmp(username, ht->map[hash_val].username) == 0) {
    return hash_val;
  }
  // Case 3: taken spot (tombstone or active object)
  else {
    // Quadratic probing
    int i = 1;
    int index = hash_val + i;
    while (index != hash_val) {
      if (ht->map[hash_val].tombstone == false && strcmp(username, ht->map[hash_val].username) == 0) {
        return index;
      }
      i++;
      index = (index + i * i) % ht->size;
    }
  }

  return -1;
}

// TODO: Resizing

void resize(hashtable_t *ht) {

}

// TODO: Insertion

void insert(hashtable_t *ht, userdata_t data) {
  if ((ht->n_elements + 1) / (double) ht->size >= LOAD_FACTOR) {
    resize(ht);
  }
}

// TODO: Deletion

int main() {
  sieve_primes();
  if (!global_primes_calculated) {
    return 1;
  }
  hashtable_t ht = generate_hashmap();

  free_hashmap(&ht);
  free(global_primes);
  global_primes = NULL;
  return 0;
}
