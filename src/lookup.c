
/*
 * This file implements the minimal lookup server
 * required by this project. It enables users to
 * search for users they want to message.
 *
 * Implements a hash table and HTTP server.
 * Uses open addressing in the hash table.
 */

#include "lookup.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char global_table_filename[256] = { '\0' };

void generate_table_filename() {
  const char *home_dir = getenv("HOME");
  size_t len = strlen(home_dir);

  memcpy(global_table_filename, home_dir, len);
  global_table_filename[255] = '\0';

  size_t dir_len = strlen(DATA_DIR);
  memcpy(global_table_filename + len, DATA_DIR, dir_len);
  global_table_filename[255] = '\0';

  mkdir(global_table_filename, 0755);

  memcpy(global_table_filename + len + dir_len, STORAGE_FILE, strlen(STORAGE_FILE));
  global_table_filename[255] = '\0';
}

/*
 * Writes the given hash table's data to the disk.
 * Creates the file if it doesn't exist. Overwrites the existing data.
 * Throws an assertion if the table file cannot be opened or if a write
 * error occurs.
 */

void write_table(hashtable_t *ht) {
  if (global_table_filename[0] == '\0') {
    generate_table_filename();
  }
  FILE *file = fopen(global_table_filename, "w");
  assert(file != NULL);

  fwrite(&ht->n_elements, sizeof(size_t), 1, file);
  for (size_t i = 0; i < ht->size; i++) {
    if (ht->map[i].tombstone == false && ht->map[i].username[0] != '\0') {
      size_t status_code = fwrite(&ht->map[i], sizeof(userdata_t), 1, file);
      assert(status_code == 1);
    }
  }

  fclose(file);
  file = NULL;
}

/*
 * Generates the UserData hashmap. Throws an
 * assertion if a memory allocation error occurs, if the table
 * file cannot be opened, or if a read error occurs. Call free_hashmap()
 * afterwards to avoid memory leaks! Uses the table in the disk if it exists.
 * Creates the file otherwise.
 */

hashtable_t generate_hashmap() {
  if (global_table_filename[0] == '\0') {
    generate_table_filename();
  }

  FILE *file = fopen(global_table_filename, "r");
  if (file == NULL) {
    file = fopen(global_table_filename, "w");
    assert(file != NULL);
    if (file != NULL) {
      fclose(file);
      file = NULL;
    }
    userdata_t *map = calloc(INITIAL_TABLE_SIZE, sizeof(userdata_t));
    assert(map != NULL);
    return (hashtable_t){ .map = map, .size = INITIAL_TABLE_SIZE, .n_elements = 0 };
  }

  hashtable_t ht;
  size_t n_users = 0;
  size_t status_code = fread(&n_users, sizeof(size_t), 1, file);
  assert(status_code == 1);

  userdata_t *map;
  if (n_users != 0) {
    map = calloc(n_users, sizeof(userdata_t));
    assert(map != NULL);
    ht.size = n_users;
    ht.n_elements = n_users;
    ht.map = map;
    for (size_t i = 0; i < n_users; i++) {
      userdata_t temp;
      status_code = fread(&temp, sizeof(userdata_t), 1, file);
      assert(status_code == 1);
      insert(&ht, temp);
    }
  } else {
    map = calloc(INITIAL_TABLE_SIZE, sizeof(userdata_t));
    assert(map != NULL);
    ht.map = map;
    ht.size = INITIAL_TABLE_SIZE;
    ht.n_elements = 0;
  }
  
  fclose(file);
  file = NULL;
  return ht;
}

void free_hashmap(hashtable_t *hm) {
  free(hm->map);
  hm->map = NULL;
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
  if (ht->map[hash_val].tombstone == false && ht->map[hash_val].username[0] == '\0') {
    return -1;
  }
  else {
    // Quadratic probing
    int i = 1;
    int index = hash_val;
    do {
      if (ht->map[index].tombstone == false && strcmp(username, ht->map[index].username) == 0) {
        return index;
      }
      index = (index + i * i) % ht->size;
      i++;
    } while (index != hash_val);
  }

  return -1;
}

/*
 * Resizes the given hash table's array by the compile-time
 * constant RESIZE_FACTOR. Sets size to MAX_TABLE_SIZE if resizing by
 * RESIZE_FACTOR causes the table to have a larger length than MAX_TABLE_SIZE.
 * Doesn't do anything if the hashtable's size is already MAX_TABLE_SIZE.
 * Throws an assertion if the passed parameter is NULL, if a memory allocation
 * error occurs, or if rehashing all elements fails.
 */

void resize(hashtable_t *ht) {
  assert(ht != NULL);

  size_t old_size = ht->size; 

  // This is never going to happen, so something's wrong if it executes
  if (ht->size == MAX_TABLE_SIZE) {
    return;
  }
  // Overflow, I expect this to never occur
  else if (ht->size * RESIZE_FACTOR > MAX_TABLE_SIZE) {
    ht->size = MAX_TABLE_SIZE;
  } else {
    ht->size *= RESIZE_FACTOR;
  }

  userdata_t *temp_map = calloc(ht->size, sizeof(userdata_t));
  hashtable_t temp_table = { .map = temp_map, .size = ht->size, .n_elements = 0 };
  for (size_t i = 0; i < old_size; i++) {
    if (ht->map[i].tombstone == false && ht->map[i].username[0] != '\0') {
      int status_code = insert(&temp_table, ht->map[i]);
      assert(status_code != -1);
    }
  }

  free(ht->map);
  ht->map = temp_map;
  ht->n_elements = temp_table.n_elements;
}

/*
 * Inserts the given data to the hash table. Resizes the
 * table if necessary. Throws an assertion if the passed
 * parameters are NULL or invalid. Returns 0 on success,
 * 1 on failure. If the given user's username already exists,
 * updates the existing data with the new data.
 */

int insert(hashtable_t *ht, userdata_t data) {
  assert(ht != NULL && data.username != NULL && data.tombstone == false);

  // If new point exists, update
  int existing_index = get_index(ht, data.username);
  if (existing_index != -1) {
    ht->map[existing_index] = data;
    return 0;
  }

  if ((ht->n_elements + 1) / (double) ht->size >= LOAD_FACTOR) {
    resize(ht);
  }

  size_t name_len = strlen(data.username);
  int hash_val = 0;
  for (size_t i = 0; i < name_len; i++) {
    hash_val += ((int) data.username[i]) * pow(HASH_PRIME, i);
  }
  hash_val %= ht->size;

  // Quadratic probing
  int i = 1;
  int index = hash_val;
  do {
    // Empty spot found
    if (ht->map[index].tombstone == true || ht->map[index].username[0] == '\0') {
      ht->map[index] = data;
      ht->n_elements++;
      return 0;
    }
    index = (index + i * i) % ht->size;
    i++;
  } while (index != hash_val);
  return -1;
}

/*
 * Finds and *LAZILY* deletes the given user from the given hash table.
 * Returns -1 if the given user does not exist in the hashmap.
 * Returns 0 on success. Throws an assertion if any of the
 * parameters are NULL.
 */

int delete_data(hashtable_t *ht, const char *username) {
  assert(ht != NULL && username != NULL);
  int index = get_index(ht, username);
  if (index == -1) {
    return -1;
  }
  ht->map[index].tombstone = true;
  ht->n_elements--;
  return 0;
}

int main() {
  generate_table_filename();
  if (global_table_filename[0] == '\0') {
    return 1;
  }
  sieve_primes();
  if (!global_primes_calculated) {
    return 1;
  }

  hashtable_t ht = generate_hashmap();
  write_table(&ht);

  free_hashmap(&ht);
  free(global_primes);
  global_primes = NULL;
  return 0;
}
