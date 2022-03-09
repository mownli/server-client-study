#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>


int main(int argc, char **argv)
{
	struct sockaddr_in clnt;

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		return 1;
	}

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	clnt.sin_family = AF_INET;
	clnt.sin_port = htons(7654);
	clnt.sin_addr.s_addr = inet_addr("127.0.0.1");

	printf("Connect = %d\n", connect(fd, (struct sockaddr*) &clnt, sizeof(clnt)));

	// Tests

	// 1
	//char a[] = "01234\n";
	//write(fd, a, sizeof(a) - 1);

	// 2
	//char a[] = "01\n23\n";
	//write(fd, a, sizeof(a) - 1);

	// 3
	//char a[] = "012345\n";
	//write(fd, a, sizeof(a) - 1);

	// 4
	//char a[] = "0123\n0";
	//write(fd, a, sizeof(a) - 1);
	//char b[] = "12345\n";
	//write(fd, b, sizeof(b) - 1);

	// 5
	//char a[] = "0123\n0";
	//write(fd, a, sizeof(a) - 1);
	//char b[] = "1234\n";
	//write(fd, b, sizeof(b) - 1);

	// 6
	//char a[] = "0123\n4";
	//write(fd, a, sizeof(a) - 1);
	//char b[] = "56";
	//write(fd, b, sizeof(b) - 1);
	//char c[] = "78\n";
	//write(fd, c, sizeof(c) - 1);

	// 7
	//char a[] = "0123\n4";
	//write(fd, a, sizeof(a) - 1);
	//char b[] = "56";
	//write(fd, b, sizeof(b) - 1);
	//char c[] = "78\n";
	//write(fd, c, sizeof(c) - 1);

	// 8
	//char a[] = "\n\n";
	//write(fd, a, sizeof(a) - 1);

	// 9
	//char a[] = "0123\n4";
	//write(fd, a, sizeof(a) - 1);
	//char b[] = "\n";
	//write(fd, b, sizeof(b) - 1);


	pause();
	return 0;
}
