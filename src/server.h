#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

typedef struct CommunicationSockets {
  int server_socket;
  int client_socket;
} comms_t;

comms_t *initialize_server();

void send_message(comms_t *, const char *);

void deinitialize_server(comms_t *);

#endif
