#include "server_head.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <openssl/err.h>

/*
	'head' points to current unclaimed buffer space
	'tail' moves with every acquired (terminated) message
	if 'tail' == 0: no message has been confirmed during this call
	'end' points to a space right after new piece of information
	Main loop starts from unparsed piece ('head') and looks for '\n' and
	writes 0 in its place, 'tail' is moved accordingly.
	If buffer (claimed) is not terminated then the residue is moved to
	unlaimed space at the start (new 'head') of the buffer
*/

int server_read(struct client_t *clnt)
{
	assert(clnt);

#ifndef NDEBUG
	printf("server_read()\n{\n");
#endif

	//if(server_ssl_handshake(clnt) == SSL_HS_PENDING) {
//#ifndef NDEBUG
		//printf("}\n");
//#endif
		//return 0;
	//}

	size_t head = clnt->buf_in.head;

	int count = 0;

	//if(clnt->ssl) {
		//server_ssl_handshake(clnt);
		count = SSL_read(clnt->ssl, &clnt->buf_in.data[head], BUF_IN_SIZE - head);

		if(count <= 0) {
			int err = SSL_get_error(clnt->ssl, count);
			if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
#ifndef NDEBUG
				printf("SSL_read() AGAIN\n");
				printf("}\n");
#endif
				return 0;
			}
			else {
				// FATAL ERROR
				clnt->buf_in.head = 0;
				ERR_print_errors_fp(stderr);
				return 1;
			}
		}

#ifndef NDEBUG
	printf("  clnt->head = %ld\n", clnt->buf_in.head);
	printf("  count = %d\n", count);
	printf("  Bytes after read():\n  ");
	for(int i = 0; i < DEBUG_MEM_PRINT - 1; i++) {
		printf("%d ", clnt->buf_in.data[i]);
	}
	printf("\n");
#endif

	// Main loop
	size_t end = head + count;
	size_t tail = 0;
	for(size_t i = head; i < end; i++) {
		if(clnt->buf_in.data[i] == '\n') {
			clnt->buf_in.data[i] = 0;
			printf("[CLIENT] <%s> %s\n", clnt->ip, &clnt->buf_in.data[tail]);
			tail = i + 1;
			head = 0;
		}
	}

#ifndef NDEBUG
	printf("  Current head = %ld\n", head);
	printf("  end = %ld\n", end);
	printf("  tail = %ld\n", tail);
#endif

	// All is read
	if(tail == end) {
		clnt->buf_in.head = 0;
		return 0;
	}
	// Too much
	if(tail == 0 && end == BUF_IN_SIZE) {
		printf("FAILURE!\n");
		fprintf(stderr, "[CLIENT] %s exceeded BUF_IN_SIZE\n", clnt->ip);
		server_send(clnt, EXCEED_MSG, EXCEED_MSG_SIZE);
		clnt->buf_in.head = 0;
		return 2;
	}

	// NOT OVER YET
	printf("NOT OVER YET!\n");
	if(tail != 0)
		memmove(&clnt->buf_in.data[head], &clnt->buf_in.data[tail], end - tail);
	clnt->buf_in.head = end - tail;

#ifndef NDEBUG
	printf("  Bytes after memmove():\n  ");
	for(int i = 0; i < DEBUG_MEM_PRINT - 1; i++) {
		printf("%d ", clnt->buf_in.data[i]);
	}
	printf("\n");
	printf("  New clnt->head = %ld\n", clnt->buf_in.head);
	printf("}\n");
#endif

	return 0;
}
