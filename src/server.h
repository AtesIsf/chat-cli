#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

// TODO: Give users an option to change these ports later

#define SERVER_PORT (50505)
#define CLIENT_PORT (50506)

typedef struct CommunicationSockets {
  int server_socket;
  int client_socket;
} comms_t;

comms_t *initialize_server();

void send_message(comms_t *, const char *);

void deinitialize_server(comms_t *);

#endif
