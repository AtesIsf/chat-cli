
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

#define INITIAL_MAP_SIZE (32)
#define HASH_PRIME (17)

typedef struct UserData {
  char public_key; // convert to RSA struct after implementing key pair
  char *username;
  char *ip; // May change later
  bool tombstone;
} userdata_t;

typedef struct HashMap {
  userdata_t *map;
  size_t size;
} hashmap_t;

/*
 * Generates the UserData hashmap. Throws an
 * assertion if a memory allocation error occurs.
 * Call free_hashmap() afterwards to avoid memory leaks!
 */

hashmap_t generate_hashmap() {
  userdata_t *map = calloc(INITIAL_MAP_SIZE, sizeof(userdata_t));
  assert(map != NULL);
  hashmap_t hm = (hashmap_t){ .map = map, .size = INITIAL_MAP_SIZE };
  return hm;
}

void free_hashmap(hashmap_t *hm) {
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

int get_index(hashmap_t *hm, const char *username) {
  assert(hm != NULL && username != NULL && hm->map != NULL);

  size_t name_len = strlen(username);
  int hash_val = 0;
  for (size_t i = 0; i < name_len; i++) {
    hash_val += ((int) username[i]) * pow(HASH_PRIME, i);
  }
  hash_val %= hm->size;

  // Case 1: does not exist
  if (hm->map[hash_val].tombstone == false && hm->map[hash_val].username == NULL) {
    return -1;
  }
  // Case 2: found
  else if(hm->map[hash_val].tombstone == false && strcmp(username, hm->map[hash_val].username) == 0) {
    return hash_val;
  }
  // Case 3: taken spot (tombstone or active object)
  else {
    // Quadratic probing
    int i = 1;
    int index = hash_val + i;
    while (index != hash_val) {
      if (hm->map[hash_val].tombstone == false && strcmp(username, hm->map[hash_val].username) == 0) {
        return index;
      }
      i++;
      index = (index + i * i) % hm->size;
    }
  }

  return -1;
}

int main() {
  hashmap_t hm = generate_hashmap();
  free_hashmap(&hm);
  return 0;
}
