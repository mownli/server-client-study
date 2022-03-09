#ifndef SERVER_OPENSSL_H
#define SERVER_OPENSSL_H

#include <openssl/ssl.h>

void init_openssl();

void cleanup_openssl();

SSL_CTX *create_context();

int configure_context(SSL_CTX *ctx);

#endif

