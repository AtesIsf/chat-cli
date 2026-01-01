#ifndef CHAT_SHARED_PROTOCOL_H
#define CHAT_SHARED_PROTOCOL_H

#include <netinet/in.h>
#include <sys/socket.h>

#define METHOD_UPDATE ('U')
#define METHOD_FETCH ('F')
#define ERR_RESPONSE ("E\0")
#define OK_RESPONSE ("K\0")

#define LOOKUP_PORT (56732)

typedef struct IPAddress {
  sa_family_t family;
  union {
    struct in_addr v4;
    struct in6_addr v6;
  } addr;
} ip_addr_t;

#endif
