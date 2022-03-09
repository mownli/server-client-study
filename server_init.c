#include "server_head.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <assert.h>
#include "server_openssl.h"


static int srv_socket_in_init(struct server_t *srv)
{
	int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if(fd == -1) {
		perror("SOCKET");
		return -1;
	}
	srv->fd = fd;

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&srv->addr, 0, sizeof(srv->addr));
	srv->addr.sin_family = AF_INET;
	srv->addr.sin_port = htons(PORT);
	srv->addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(fd, (struct sockaddr*) &(srv->addr), sizeof(srv->addr))) {
		perror("BIND");
		return -1;
	}

	if(listen(fd, LISTEN_QLEN)) {
		perror("LISTEN");
		return -1;
	}

	return fd;
}

static int srv_socket_un_init(struct server_t *srv)
{
	unlink(UNIX_SOCKET_PATH);
	// DGRAM is one-way
	int fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if(fd == -1) {
		perror("SOCKET");
		return -1;
	}
	srv->fd_un = fd;

	memset(&srv->addr_un, 0, sizeof(srv->addr_un));
	srv->addr_un.sun_family = AF_UNIX;
	strncpy(srv->addr_un.sun_path, UNIX_SOCKET_PATH, sizeof(srv->addr_un.sun_path) - 1);
	if(bind(fd, (struct sockaddr*)&srv->addr_un, sizeof(struct sockaddr_un))) {
		perror("BIND");
		return -1;
	}

	return fd;
}

static int srv_clients_init(struct server_t *srv)
{
	struct client_t *clients = NULL;
	clients = malloc(sizeof(struct client_t) * INIT_SSN_ARR_N);
	if(!clients) {
		perror("ARRAY MALLOC");
		return 1;
	}
	srv->clients = clients;

	memset(clients, 0, sizeof(struct client_t) * INIT_SSN_ARR_N);
	for(int i = 0; i < INIT_SSN_ARR_N; i++)
		clients[i].fd = -1;

	srv->clients_size = INIT_SSN_ARR_N;

	//for(int i = 0; i < INIT_SSN_ARR_N; i++) {
		//SSL *ssl = SSL_new(srv->ctx);
		//if(!ssl) {
			//perror("SSL NEW");
			//return 1;
		//}
		//srv->clients[i].ssl = ssl;
	//}

	return 0;
}

static int signalfd_init(struct server_t *srv)
{
	// Signalfd (not eligible for multithreading)
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGQUIT);

	if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
		perror("SIGPROCMASK");
		return -1;
	}

	int fd = signalfd(-1, &mask, 0);
	if(fd == -1) {
		perror("SIGNALFD");
		return -1;
	}

	srv->fd_sig = fd;
	return fd;
}

int server_init(struct server_t *srv)
{
	assert(srv);

	int fd = -1;

	srv->timeout = -1;

	fd = srv_socket_in_init(srv);
	if(fd == -1)
		return 1;

	fd = srv_socket_un_init(srv);
	if(fd == -1)
		return 1;

	// OpenSSL context
	SSL_CTX *ctx = NULL;
    init_openssl();
    ctx = create_context();
    if(!ctx)
		return 1;
	srv->ctx = ctx;
	if(configure_context(ctx))
		return 1;

	if(srv_clients_init(srv))
		return 1;

	fd = signalfd_init(srv);
	if(fd == -1)
		return 1;

	// Choose the max listener fd, needed for calculations
	srv->max_lstn_fd = fd;
	//srv->maxfd = fd;
	//srv->maxfd_index = 0;

	return 0;
}
