
/*
 * This file implements the minimal lookup server
 * required by this project. It enables users to
 * search for users they want to message.
 *
 * Implements a hash table and HTTP server.
 * Uses open addressing in the hash table.
 */

#include "lookup.h"
#include "ssl.h"

#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

char global_table_filename[256] = { '\0' };

volatile bool global_terminate_program = false;

void terminate_signal(int n) {
  global_terminate_program = true;
}

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

/*
 * Handles the given fetch request. Returns NULL if the
 * given username doesn't exist in the hash table.
 * Throws an assertion if any of the parameters are NULL or
 * if a heap allocation error occurs. Returns a heap-allocated response string. 
 * Expected format: "method char|username"
 * Returned format: "4 or 6|ip address"
 */

char *handle_fetch(const char *msg, hashtable_t *ht) {
  char buf[MAX_USERNAME_LEN] = { '\0' };
  int status_code = sscanf(msg, "%*c|%31[^\n]", buf);
  buf[31] = '\0';
  // The discarded char is not counted
  if (status_code != 1) {
    return NULL;
  }
  int index = get_index(ht, buf);
  if (index == -1) {
    return NULL;
  }
  ip_addr_t ip = ht->map[index].ip;

  const void *src = NULL;
  size_t len = 0;
  char type_char = 'X';
  if (ip.family == AF_INET) {
    src = &ip.addr.v4;
    len = INET_ADDRSTRLEN;
    type_char = '4';
  } else if (ip.family == AF_INET6) {
    src = &ip.addr.v6;
    len = INET6_ADDRSTRLEN;
    type_char = '6';
  } else {
    return NULL;
  }
  
  char *ip_str = calloc(sizeof(char), len);
  assert(ip_str != NULL);
  inet_ntop(ip.family, src, ip_str, len);
  char *response = malloc(sizeof(char) * len + 4);
  assert(response != NULL);
  sprintf(response, "%c|%s\n", type_char, ip_str);
  response[len + 4] = '\0';
  free(ip_str);

  return response;
}

/*
 * Handles an update request (record new user or update existing).
 * Throws an assertion if any of the parameters are NULL or if a
 * heap allocation error occurs. Returns a heap-allocated response
 * string.
 * Expected format: "4 or 6|username|ip address"
 * Returned format: "E (error) or K (ok)"
 */

char *handle_update(const char *msg, hashtable_t *ht) {
  userdata_t data = { 0 };
  data.tombstone = false;
  char ip_type = '\0';
  char ip_addr[INET6_ADDRSTRLEN + 1] = { '\0' };

  int status_code = sscanf(msg, "%c|%31[^|]|%46[\n]", &ip_type, data.username, ip_addr);
  if (status_code != 3) {
    return strdup(ERR_RESPONSE);
  }
  if (ip_type == '4') {
    data.ip.family = AF_INET;
    inet_net_pton(data.ip.family, ip_addr, &data.ip.addr.v4, INET_ADDRSTRLEN);
  } else if (ip_type == '6') {
    data.ip.family = AF_INET6;
    inet_net_pton(data.ip.family, ip_addr, &data.ip.addr.v6, INET6_ADDRSTRLEN);
  } else {
    return strdup(ERR_RESPONSE);
  }
  int result = insert(ht, data);
  return result == 0 ? strdup(OK_RESPONSE) : strdup(ERR_RESPONSE);
}

/*
 * Initializes an input using given SSL_CTX pointer and
 * routes the received request to the relevant handler.
 * Throws an assertion if any of the parameters are NULL.
 */

void endpoint_manager(SSL_CTX *ctx, hashtable_t *ht) {
  assert(ctx != NULL && ht != NULL);
  signal(SIGINT, terminate_signal);

  int endpoint = socket(AF_INET6, SOCK_STREAM, 0);
  if (endpoint == 0) {
    fprintf(stderr, "[ERROR] Failed initializing endpoint socket (%d)\n", errno);
    return;
  }

  int opt = 1;
  if (setsockopt(endpoint, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
    fprintf(stderr, "[ERROR] setsockopt() failed (%d)\n", errno);
    return;
  }

  struct sockaddr_in6 addr = { 0 };
  int addr_size = sizeof(addr);
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;
  addr.sin6_port = htons(PORT);

  if (bind(endpoint, (struct sockaddr *) &addr, (socklen_t) addr_size) < 0) {
    fprintf(stderr, "[ERROR] bind() failed (%d)\n", errno);
    return;
  }

  if (listen(endpoint, 10) < 0) {
    fprintf(stderr, "[ERROR] listen() failed (%d)\n", errno);
    return;
  }
  printf("[INFO] Listening on port %d...\n", PORT);


  while (!global_terminate_program) {
    int handler_fd = accept(endpoint, (struct sockaddr *) &addr, (socklen_t *) &addr_size);
    if (handler_fd < 0) {
      continue;
    }

    SSL *ssl = SSL_new(ctx);
    if (SSL_set_fd(ssl, handler_fd) != 1) {
      close(handler_fd);
      SSL_shutdown(ssl);
      SSL_free(ssl);
      continue;
    }
    if (SSL_accept(ssl) <= 0) {
      close(handler_fd);
      SSL_shutdown(ssl);
      SSL_free(ssl);
      continue;
    }

    char buf[1024] = { '\0' };
    int bytes_read = SSL_read(ssl, buf, sizeof(buf) - 1);
    buf[bytes_read] = '\0';
    if (bytes_read == 0) {
      close(handler_fd);
      SSL_shutdown(ssl);
      SSL_free(ssl);
      continue;
    }
    char method = '\0';
    int status_code = sscanf(buf, "%c", &method);
    if (status_code != 0) {
      close(handler_fd);
      SSL_shutdown(ssl);
      SSL_free(ssl);
      continue;
    }
    char *response = NULL;
    if (method == METHOD_UPDATE) {
      response = handle_update(buf, ht);
    } else if (method == METHOD_FETCH) {
      response = handle_fetch(buf, ht);
    }
    
    if (response != NULL) {
      SSL_write(ssl, response, strlen(response));
      free(response);
      response = NULL;
    } else {
      SSL_write(ssl, ERR_RESPONSE, strlen(ERR_RESPONSE));
    }
    close(handler_fd);
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  close(endpoint);
}

int main() {
  generate_table_filename();
  get_cert_dirs();
  SSL_CTX *ctx = init_openssl();
  if (ctx == NULL) {
    return 1;
  }
  if (global_table_filename[0] == '\0') {
    return 1;
  }

  hashtable_t ht = generate_hashmap();
  write_table(&ht);

  SSL_CTX_free(ctx);
  free_hashmap(&ht);
  return 0;
}
