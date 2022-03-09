#include "server_head.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int server_remove_client(struct server_t *srv, int fd)
{
	int index = fd - srv->max_lstn_fd - 1;

	if(srv->clients[index].ssl) {
		SSL_shutdown(srv->clients[index].ssl);
		SSL_free(srv->clients[index].ssl);
		srv->clients[index].ssl = NULL;
    }

	int res = close(fd);

#ifndef NDEBUG
	printf("server_remove_client()\n{\n");
	printf("  Closing %d\n", fd);
	printf("  Result: %d\n", res);
	printf("}\n");
#endif

	printf("[SRV] Dropped %s\n", srv->clients[index].ip);

	memset(&srv->clients[index], 0, sizeof(struct client_t));
	srv->clients[index].fd = -1;

	return 0;
}

