#include "server_head.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>


static int array_resize(struct server_t *srv)
{
	size_t newlen = srv->clients_size * 2;
	srv->clients = realloc(srv->clients, newlen * sizeof(struct client_t));
	if(errno == ENOMEM) {
		perror("[ERROR] realloc()");
		return 1;
	}
	memset(&srv->clients[srv->clients_size], 0, sizeof(struct client_t) * srv->clients_size);
	for(int i = srv->clients_size; i < newlen; i++) {
		srv->clients[i].fd = -1;
	}
	srv->clients_size = newlen;

	//for(size_t i = srv->clients_size; i < newlen; i++) {
		//SSL *ssl = SSL_new(srv->ctx);
		//if(!ssl) {
			//// Free previosuly allocated stuff
			//for(size_t k = srv->clients_size; k < i; k++) {
				//free(srv->clients[k].ssl);
			//}
			//perror("[ERROR] SSL NEW");
			//return 1;
		//}
		//srv->clients[i].ssl = ssl;
	//}

	return 0;
}

int server_accept(struct server_t *srv)
{
	assert(srv);

	struct sockaddr_in addr = {};
	socklen_t len = sizeof(struct sockaddr_in);
	int sd = accept(srv->fd, (struct sockaddr*) &addr, &len);
	if(sd == -1) {
		perror("[ERROR] accept()");
		return -1;
	}

	// O_NONBLOCK gets removed after handshake
	int flags = fcntl(sd, F_GETFL);
	fcntl(sd, F_SETFL, flags | O_NONBLOCK);

	size_t fd_index = sd - srv->max_lstn_fd - 1;

	// Array resizing
	if(fd_index >= srv->clients_size)
		if(array_resize(srv))
			goto cleanup;

	// Writing new values and decreasing free space by one
	srv->clients[fd_index].fd = sd;
	srv->clients[fd_index].addr = addr;
	strcpy(srv->clients[fd_index].ip, inet_ntoa(addr.sin_addr)); // Static buf inside
	srv->clients[fd_index].port = ntohs(addr.sin_port);

	// OpenSSL
	SSL *ssl = SSL_new(srv->ctx);
	if(!ssl) {
		perror("[ERROR] SSL NEW");
		goto cleanup;
	}
	srv->clients[fd_index].ssl = ssl;
	SSL_set_fd(ssl, sd); // Return 1 on success

	SSL_set_accept_state(ssl);
	server_ssl_handshake(&srv->clients[fd_index]);

	//if(sd > srv->maxfd) {
		//srv->maxfd = sd;
		//srv->maxfd_index = sd - srv->max_lstn_fd;
	//}
	srv->fd_num++;

	printf(
		"[SRV] New client from %s bound to port %d\n",
		srv->clients[fd_index].ip,
		srv->clients[fd_index].port);

#ifndef NDEBUG
	printf("server_accept()\n{\n");
	printf(
		"  srv->clients_size = %ld\n  srv->fd_num = %d\n",
		srv->clients_size, srv->fd_num);
	printf("  Client FDs:\n");
	for(size_t i = 0; i < srv->clients_size ; i++)
		printf("  %d\n", srv->clients[i].fd);
	printf("}\n");
#endif

	return sd;

cleanup:
	close(sd);
	if(ssl) {
		SSL_free(ssl);
		srv->clients[fd_index].ssl = NULL;
	}
	return -1;
}
