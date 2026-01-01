#include "ssl.h"

#include <assert.h>
#include <openssl/crypto.h>
#include <openssl/core.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/provider.h>
#include <netinet/in.h>
#include <sys/stat.h>

char global_cert_path[256] = { '\0' };
char global_privkey_path[256] = { '\0' };

/*
 * Populates the global arrays that hold the file paths
 * to the private key and openssl certificate.
 * Throws an assertion if any one is already populated.
 */

void get_cert_dirs() {
  assert(global_cert_path[0] == '\0' && global_privkey_path[0] == '\0');

  const char *home_dir = getenv("HOME");
  size_t len = strlen(home_dir);

  memcpy(global_privkey_path, home_dir, len);
  global_privkey_path[255] = '\0';

  size_t dir_len = strlen(DATA_DIR);
  memcpy(global_privkey_path + len, DATA_DIR, dir_len);
  global_privkey_path[255] = '\0';

  mkdir(global_privkey_path, 0755);

  memcpy(global_cert_path, global_privkey_path, strlen(global_privkey_path));
  global_cert_path[255] = '\0';

  memcpy(global_privkey_path + len + dir_len, PRIVKEY_FILE, strlen(PRIVKEY_FILE));
  global_privkey_path[255] = '\0';

  memcpy(global_cert_path + len + dir_len, CERT_FILE, strlen(CERT_FILE));
  global_cert_path[255] = '\0';
}

/*
 * Initializes the OPENSSL context and returns a SSL_CTX pointer.
 * The pointer must be freed later. Returns NULL if it fails to initialize.
 */

SSL_CTX *init_openssl(enum ContextMode mode) {
  OPENSSL_init_ssl(0, NULL);
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();

  SSL_CTX *ctx = NULL;
  if (mode == SERVER) {
    ctx = SSL_CTX_new(TLS_server_method());
  } else if (mode == CLIENT) {
    ctx = SSL_CTX_new(TLS_client_method());
  } else {
    return NULL;
  }

  SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4");
  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
  SSL_CTX_set_ecdh_auto(ctx, 1);
  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

  if (SSL_CTX_use_certificate_file(ctx, global_cert_path, SSL_FILETYPE_PEM) < 0) {
    puts("[ERROR] Required files do not exist! Run 'make keygen' in the project root to fix this.");
    SSL_CTX_free(ctx);
    ctx = NULL;
    return ctx;
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, global_privkey_path, SSL_FILETYPE_PEM) < 0) {
    puts("[ERROR] Required files do not exist! Run 'make keygen' in the project root to fix this.");
    SSL_CTX_free(ctx);
    ctx = NULL;
    return ctx;
  }
  return ctx;
}

