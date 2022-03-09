#include "server_head.h"
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <poll.h>

#include <pthread.h>

static int add_pfd(
	struct server_t *srv,
	struct pollfd **pfds,
	int fd,
	size_t *pfds_size)
{
	size_t fd_index = fd - srv->fd;
	if(fd_index >= *pfds_size)
	{
		size_t oldsize = *pfds_size;
		*pfds_size *= 2;
		*pfds = realloc(*pfds, *pfds_size * sizeof(struct pollfd));
		if(errno == ENOMEM)
		{
			perror("[ERROR] realloc()");
			return 1;
		}

		for(int i = oldsize + 1; i < *pfds_size; i++)
			(*pfds)[i].fd = -1;
	}

	(*pfds)[fd_index].fd = fd;
	(*pfds)[fd_index].events = POLLIN;

	srv->fd_num++;

	return 0;
}

//static long long unsigned fib(unsigned x)
//{
	//if(x == 0 || x == 1)
	//{
		//return 1;
	//}
	//else
		//return fib(x - 1) + fib(x - 2);
//}

struct arg_t {
	struct client_t *clnt;
	unsigned fibnum;
	size_t size;
};

static void *thread(void *arg)
{
	char bb[100] = {};
	//fib(*(unsigned*)arg)
	//sprintf(bb, "%llu safdss", fib(((struct arg_t*)arg)->fibnum));
	sprintf(bb, "a%u", ((struct arg_t*)arg)->fibnum);
	int ret = server_send(((struct arg_t*)arg)->clnt, bb, ((struct arg_t*)arg)->size);
	if(ret < 0)
		exit(1);
	//printf("SEND RET = %d\n", ret);
	return NULL;
}

static struct client_t *fd_to_clnt(struct server_t *srv, int fd)
{
	return &srv->clients[fd - srv->max_lstn_fd - 1];
}

int server_main_loop(struct server_t *srv)
{
	assert(srv);

	// Return value
	int ret = 0;

	size_t pfds_size = INIT_PFDS_N;
	struct pollfd *pfds = malloc(pfds_size * sizeof(struct pollfd));
	if(!pfds)
	{
		perror("[ERROR] malloc()");
		return 1;
	}
	for(int i = 0; i < pfds_size; i++)
		pfds[i].fd = -1;

	if(add_pfd(srv, &pfds, srv->fd, &pfds_size)) { ret = 1; goto cleanup; }
	if(add_pfd(srv, &pfds, srv->fd_un, &pfds_size)) { ret = 1; goto cleanup; }
	if(add_pfd(srv, &pfds, srv->fd_sig, &pfds_size)) { ret = 1; goto cleanup; }

	for(;;) {

#ifndef NDEBUG
		printf("main_loop()\n{\n");
		printf(
			"  srv->clients_size = %ld\n  srv->max_lstn_fd = %d\n  srv->fd_num = %d\n",
			srv->clients_size, srv->max_lstn_fd, srv->fd_num);
		printf("  Client FDs:\n");
		for(size_t i = 0; i < srv->clients_size; i++)
			printf("  %d", srv->clients[i].fd);
		printf("\n  pfds:\n");
		for(size_t k = 0; k < pfds_size; k++)
			printf("  %d", pfds[k].fd);
		printf("\n}\n");
#endif

		int timeout = srv->timeout;

		int res = poll(pfds, pfds_size, timeout);

		if(res == -1)
		{
			if(errno != EINTR)
			{
				perror("[ERROR] select()");
				ret = 1;
				goto cleanup;
			}
			// Signal handling
		}
		else if(res == 0)
		{
			// Timeout handling
			for(int i = 0; i < srv->clients_size; i++)
			{
				if(srv->clients[i].fd != -1)
					if(srv->clients[i].buf_out.head != 0)
						server_send(&srv->clients[i], NULL, 0);
			}
		}
		else
		{
			int fds_found = 0;
			for(int i = 0; fds_found < res; i++)
			{
				if(pfds[i].revents & POLLIN)
				{
					if(pfds[i].fd == srv->fd)
					{
						// New connection handling
						int newfd = server_accept(srv);
						if(newfd == -1)
						{
							fprintf(stderr, "[SRV] Couldn't accept a client\n");
						}
						else
						{
							if(add_pfd(srv, &pfds, newfd, &pfds_size))
							{
								ret = 1;
								goto cleanup;
							}

							struct client_t *clnt = fd_to_clnt(srv, newfd);
							struct arg_t arg1 = {clnt, 1, 3};
							struct arg_t arg2 = {clnt, 10, 4};
							pthread_t thr;
							pthread_create(&thr, NULL, thread, &arg1);
							pthread_create(&thr, NULL, thread, &arg2);

							//thread(clnt);

							//char bb[] = "get out bitch";
							//sprintf(srv->clients[0].buf_out.data, "%s", bb);
							//srv->clients[0].buf_out.head = sizeof(bb);

							//pfds[3].events = pfds[3].events | POLLOUT;
						}
					}
					else if(pfds[i].fd == srv->fd_un)
					{
						if(server_unix_read(srv))
						{
							// Exit message received
							goto cleanup;
						}
					}
					else if(pfds[i].fd == srv->fd_sig)
					{
						/*
							If you don't plan to quit, signalfd must be
							either reset or removed from pfds
						*/
						printf("[SRV] Caught a signal. Terminating...\n");

						goto cleanup;
					}
					else
					{
						struct client_t *clnt = fd_to_clnt(srv, pfds[i].fd);
						if(server_read(clnt))
						{
							// Error. Removing the client
							if(server_remove_client(srv, pfds[i].fd))
							{
								ret = 2;
								goto cleanup;
							}
							else
							{
								pfds[i].fd = -1;
								pfds[i].revents = 0;
							}
						}
					}

					fds_found++;
				}

				// Retry to send message
				//if (pfds[i].revents & POLLOUT)
				//{
					//struct client_t *clnt = fd_to_clnt(srv, pfds[i].fd);
					//int retry = server_send(clnt);
					//if(retry < 0)
					//{
						//// Error. Removing the client
						//if(server_remove_client(srv, pfds[i].fd))
						//{
							//ret = 2;
							//goto cleanup;
						//}
						//else
						//{
							//pfds[i].fd = -1;
							//pfds[i].revents = 0;
						//}
					//}
					//else if(retry == 0)
					//{
						//pfds[i].events = pfds[i].events & (~POLLOUT);
					//}

					//fds_found++;
				//}
			}
		}
	}

cleanup:
	free(pfds);
	return ret;
}

