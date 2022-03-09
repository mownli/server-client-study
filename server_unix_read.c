#include "server_head.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define EXIT_HEAD "exit"
#define MSGALL_HEAD "msgall="


int server_unix_read(struct server_t *srv)
{
	char buf[UNIX_BUF_SIZE]; // Initialization is probably avoidable
	int count_read = read(srv->fd_un, buf, UNIX_BUF_SIZE);
	printf("[UMSG] %s\n", buf);

	// EXIT
	if(strncmp(EXIT_HEAD, buf, sizeof(EXIT_HEAD) - 1) == 0)
		return 1;

	// MSGALL
	if(strncmp(MSGALL_HEAD, buf, sizeof(MSGALL_HEAD) - 1) == 0) {
		size_t start = sizeof(MSGALL_HEAD) - 1;
		size_t size = count_read - start;
		for(size_t i = 0; i < srv->clients_size; i++) {
			if(srv->clients[i].fd != 0) {
				server_send(&srv->clients[i], &buf[start], size);
			}
		}

		return 0;
	}

	return 0;
}

