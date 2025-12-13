#include "server.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

/*
 * Initializes and returns a struct that stores the
 * server and client sockets of the application. The
 * returned struct pointer must be deinitialized using
 * deinitialize_server() afterwards to prevent memory leaks.
 * Throws an assertion error if heap memory allocation fails.
 */

comms_t *initialize_server() {
  comms_t *comms = malloc(sizeof(comms_t));
  assert(comms != NULL);


  return comms;
}

/*
 * Sends a message to the given address. Asserts that
 * the parameters are not NULL
 */

void send_message(comms_t *comms, const char *addr) {
  assert(comms != NULL && addr != NULL);
}

void deinitialize_server(comms_t *comms) {
  assert(comms != NULL);
}

/*
 * Reads the configs from the configs file. Return a pointer to the
 * configs_t singleton that includes all user settings. Throws an assertion 
 * if memory allocation fails or if there is a file I/O error.
 * Returned pointer must be freed!
 */

configs_t *read_configs() {
  configs_t *conf = malloc(sizeof(configs_t));
  assert(conf != NULL);

  const char *home_dir = getenv("HOME");
  char data_str[256] = { '\0' };
  size_t len = strlen(home_dir);

  memcpy(data_str, home_dir, len);
  data_str[255] = '\0';

  size_t dir_len = strlen(DATA_DIR);
  memcpy(data_str + len, DATA_DIR, dir_len);
  data_str[255] = '\0';

  mkdir(data_str, 0755);

  memcpy(data_str + len + dir_len, CONFIG_FILE_NAME, strlen(CONFIG_FILE_NAME));
  data_str[255] = '\0';

  FILE *fp = fopen(data_str, "r");

  if (fp == NULL) {
    fp = fopen(data_str, "w");
    conf->client_port = DEFAULT_CLIENT_PORT;
    conf->server_port = DEFAULT_SERVER_PORT;
    int status = fwrite(conf, sizeof(configs_t), 1, fp);
    assert(status == 1);
  } else {
    int status = fread(conf, sizeof(configs_t), 1, fp);
    assert(status == 1);
  }

  if (fp != NULL) {
    fclose(fp);
  }
  fp = NULL;
  return conf;
}

