#include "server_head.h"
#include <stdio.h>
#include <fcntl.h>


enum ssl_handshake server_ssl_handshake(struct client_t *clnt)
{
	if(clnt->ssl_hs_state == SSL_HS_PENDING) {
		int ret = SSL_do_handshake(clnt->ssl);

		if(ret != 1) {
			int err = SSL_get_error(clnt->ssl, ret);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {

#ifndef NDEBUG
				printf("HANDSHAKE NOT DONE YET!\n");
#endif

				clnt->ssl_hs_state = SSL_HS_PENDING;
			}
			else {
				printf("[SRV] TLS handshake failed\n");

				SSL_free(clnt->ssl);
				clnt->ssl = NULL;

				clnt->ssl_hs_state = SSL_HS_ERROR;
			}
		}
		else {
			printf("[SRV] TLS connection establihed\n");

			//int flags = fcntl(clnt->fd, F_GETFL) & (~O_NONBLOCK);
			//fcntl(clnt->fd, F_SETFL, flags);

			clnt->ssl_hs_state = SSL_HS_OK;
		}
	}

	return clnt->ssl_hs_state;

}

