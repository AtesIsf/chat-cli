#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <openssl/crypto.h>

// The certificates will be in DATA_DIR (defined in database.h)
#define PRIVKEY_FILE ("/key.pem")
#define CERT_FILE ("/cert.pem")
#define FINGER_FILE ("/fingerprint.txt")

extern char global_cert_path[256];
extern char global_privkey_path[256];

typedef struct CommunicationSockets {
  int server_socket;
  int client_socket;
} comms_t;

void get_cert_dirs();

SSL_CTX *init_openssl();

comms_t *initialize_server();

void send_message(comms_t *, const char *);

void deinitialize_server(comms_t *);

#endif

