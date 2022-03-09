#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>


int main(int argc, char **argv)
{
	struct sockaddr_un addr_un;

	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(fd == -1) {
		return 1;
	}

	memset(&addr_un, 0, sizeof(addr_un));
	addr_un.sun_family = AF_UNIX;
	strncpy(addr_un.sun_path, "/tmp/shit.socket", sizeof(addr_un.sun_path) - 1);

	//char exit_msg[] = "exit";
	//sendto(fd, exit_msg, sizeof(exit_msg), 0, (struct sockaddr*)&addr_un, sizeof(struct sockaddr_un));

	char msg_all[] = "msgall=hi everybody";
	sendto(fd, msg_all, sizeof(msg_all), 0, (struct sockaddr*)&addr_un, sizeof(struct sockaddr_un));
	char msg_again[] = "msgall=again";
	sendto(fd, msg_again, sizeof(msg_again), 0, (struct sockaddr*)&addr_un, sizeof(struct sockaddr_un));

	//pause();
	close(fd);
	return 0;
}
