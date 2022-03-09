#include "server_head.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <openssl/err.h>


static int my_write(struct server_t *srv, int index, size_t size)
{
	if(!SSL_is_init_finished(srv->clients[index].ssl))
		return 1;

	int count = SSL_write(
		srv->clients[index].ssl,
		srv->clients[index].buf_out.data,
		size);

	if(count <= 0)
	{
		int err = SSL_get_error(srv->clients[index].ssl, count);
		if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ)
		{
			// EAGAIN
			return 1;
		}
		else
		{
			// FATAL ERROR
			ERR_print_errors_fp(stderr);
			srv->clients[index].buf_out.head = 0;
			return -1;
		}
	}

	srv->clients[index].buf_out.head = 0;
	return 0;
}

int server_send(struct client_t *clnt, const char *msg, size_t size)
{
	assert(clnt && size <= BUF_OUT_SIZE);

#ifndef NDEBUG
	printf("server_send()\n{\n");
	printf("  FD = %d\n", clnt->fd);
	printf("  buf = %s\n", msg);
	printf("  Size = %ld\n", size);
	printf("  Bytes buf:\n  ");
	for(size_t i = 0; i < clnt->buf_out.head; i++)
		printf("%d ", clnt->buf_out.data[i]);
	printf("\n");
	printf("  Bytes msg:\n  ");
	for(size_t i = 0; i < size; i++)
		printf("%d ", msg[i]);
	printf("\n");
#endif

	int ret = 0;

	if(clnt->buf_out.head)
	{
		ret = my_write(clnt, clnt->buf_out.head);

#ifndef NDEBUG
		printf("  Written: %d\n", ret);
#endif

		if(ret == -1)
			return -1;
	}

	if(!msg)
		return !(ret > 0);

	size_t space_left = BUF_OUT_SIZE - clnt->buf_out.head;
	if(size > space_left)
	{
		fprintf(stderr, "[ERROR] Output buffer is clogged\n");
		clnt->buf_out.head = 0;
		return -2;
	}

	memcpy(&clnt->buf_out.data[clnt->buf_out.head], msg, size);

	clnt->buf_out.head += size;
	clnt->buf_out.data[clnt->buf_out.head - 1] = '\n';

	ret = my_write(clnt, clnt->buf_out.head);

#ifndef NDEBUG
	printf("  Written: %d\n", ret);
	printf("}\n");
#endif

	return !(ret > 0);
}
