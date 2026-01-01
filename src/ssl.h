#ifndef CHAT_SSL_H
#define CHAT_SSL_H

#include <openssl/crypto.h>

#ifndef DATA_DIR
#define DATA_DIR ("/.chat-cli-lookup")
#endif

#define PRIVKEY_FILE ("/key.pem")
#define CERT_FILE ("/cert.pem")

extern char global_cert_path[256];
extern char global_privkey_path[256];

enum ContextMode {
  SERVER,
  CLIENT
};

void get_cert_dirs();

SSL_CTX *init_openssl(enum ContextMode);

#endif
