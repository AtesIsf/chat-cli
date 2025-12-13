#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <stddef.h>

// TODO: Randomize default values on each config generation?

#define DEFAULT_SERVER_PORT (50505)
#define DEFAULT_CLIENT_PORT (50506)

#ifndef DATA_DIR
#define DATA_DIR ("/.chat-cli")
#endif
#define CONFIG_FILE_NAME ("/config.txt")

typedef struct CommunicationSockets {
  int server_socket;
  int client_socket;
} comms_t;

typedef struct ConfigsSingleton {
  size_t server_port;
  size_t client_port;
} configs_t;

comms_t *initialize_server();

void send_message(comms_t *, const char *);

void deinitialize_server(comms_t *);

configs_t *read_configs();

#endif
