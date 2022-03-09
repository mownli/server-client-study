#include "server_head.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "server_openssl.h"

void server_cleanup(struct server_t *srv)
{
	assert(srv);

	unlink(UNIX_SOCKET_PATH);

	close(srv->fd);
	close(srv->fd_un);
	close(srv->fd_sig);

	if(srv->clients) {
		for(int i = 0; i < srv->clients_size; i++) {
			if(srv->clients[i].fd != -1)
				close(srv->clients[i].fd);

			if(srv->clients[i].ssl) {
				SSL_shutdown(srv->clients[i].ssl);
				SSL_free(srv->clients[i].ssl);
				srv->clients[i].ssl = NULL;
			}
		}

		free(srv->clients);
		srv->clients = NULL;
	}

	if(srv->ctx)
		SSL_CTX_free(srv->ctx);
    cleanup_openssl();
}
