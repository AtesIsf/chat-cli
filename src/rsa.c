
/*
 * Implements public-key cryptography, specifically RSA.
 */

#include "rsa.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

size_t *global_primes = NULL;
size_t global_n_primes = 0;
bool global_primes_calculated = false;

/*
 * Calculates all primes less than MAX_SIEVE_VAL. Sets
 * the global primes array and n_primes value.
 * Throws an assertion error on memory allocation failure.
 */

void sieve_primes() {
  bool *is_prime = malloc((MAX_SIEVE_VAL + 1) * sizeof(bool));
  assert(is_prime != NULL);
  memset(is_prime, 1, MAX_SIEVE_VAL + 1);

  for (size_t i = 2; i * i <= MAX_SIEVE_VAL; i++) {
    if (is_prime[i]) {
      for (size_t j = i * i; j <= MAX_SIEVE_VAL; j += i) {
        is_prime[j] = false;
      }
    }
  }

  global_n_primes = 0;
  for (size_t i = 2; i < MAX_SIEVE_VAL + 1; i++) {
    if (is_prime[i]) {
      global_n_primes++;
    }
  }
  global_primes = malloc(sizeof(size_t) * global_n_primes);
  assert(global_primes != NULL);
  size_t index = 0;
  for (size_t i = 2; i < MAX_SIEVE_VAL + 1; i++) {
    if (is_prime[i]) {
      global_primes[index++] = i;
    }
  }

  free(is_prime);
  is_prime = NULL;
  global_primes_calculated = true;
}

/*
 * Generates and returns an RSA key pair. Selects primes
 * randomly. Asserts that the global_primes_calculated flag
 * has been set.
 */

rsa_t rsa_keygen() {
  assert(global_primes_calculated);

  srand(time(NULL));
  /* Commented out to prevent unused variable warning
  size_t first_prime = global_primes[rand() % (global_n_primes + 1)];
  size_t second_prime = global_primes[rand() % (global_n_primes + 1)];

  size_t primes_product = first_prime * second_prime;
  */

  // TODO: continue implementation
  
  return (rsa_t){0};
}

/*
 * Encrypts the given message and returns a heap-allocated string.
 * The returned string and the passed string must be freed by
 * the called to prevent memory leaks!!! Throws an assertion on
 * heap memory allocation errors and if the given parameters are
 * invalid.
 */

const char *rsa_encrypt(size_t public_key, const char *msg) {
  return NULL;
}

/*
 * Decrypts the given message and returns a heap-allocated string.
 * The returned string and the passed string must be freed by
 * the called to prevent memory leaks!!! Throws an assertion on
 * heap memory allocation errors and if the given parameters are
 * invalid.
 */

const char *rsa_decrypt(size_t private_key, const char *secret) {
  return NULL;
}

