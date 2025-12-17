#ifndef CHAT_RSA_H
#define CHAT_RSA_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_SIEVE_VAL (1e5)

extern size_t *global_primes;
extern size_t global_n_primes;
extern bool global_primes_calculated;

typedef struct RSAKeyPair {
  size_t pub;
  size_t priv;
} rsa_t;

void sieve_primes();

rsa_t rsa_keygen();

const char *rsa_encrypt(size_t, const char *);

const char *rsa_decrypt(size_t, const char *);

#endif
