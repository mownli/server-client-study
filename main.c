#include "server_head.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	struct server_t srv = {};

	if(server_init(&srv)) {
		server_cleanup(&srv);
		return 1;
	}
	printf("[SRV] server started.\n");

	server_main_loop(&srv);

	server_cleanup(&srv);
	return 0;
}

