#ifndef SERVER_HEAD_H
#define SERVER_HEAD_H

#include <netinet/in.h>
#include <sys/un.h>
#include <openssl/ssl.h>

#define PORT 7654
#define LISTEN_QLEN 16
#define BUF_IN_SIZE 512
#define BUF_OUT_SIZE 6
#define UNIX_BUF_SIZE 100
#define INIT_SSN_ARR_N 5
#define INIT_PFDS_N 8
#define UNIX_SOCKET_PATH "/tmp/shit.socket"
#define DEFAULT_TIMEOUT 3000 // in ms

#define DEBUG_MEM_PRINT 10

// Watch the size
//#define GREET_MSG "Greetings!"
#define GREET_MSG "G!"
#define GREET_MSG_SIZE sizeof(GREET_MSG)

//#define EXCEED_MSG "Exceeded buffer limit!"
#define EXCEED_MSG "E!"
#define EXCEED_MSG_SIZE sizeof(EXCEED_MSG)


struct buf_in_t {
	char data[BUF_IN_SIZE];
	size_t head;
};

struct buf_out_t {
	char data[BUF_OUT_SIZE];
	size_t head;
};

struct client_t {
	int fd;
	struct pollfd *pfd;
	struct sockaddr_in addr;
	struct sockaddr_un addr_un;
	struct buf_in_t buf_in;
	struct buf_out_t buf_out;
	char ip[16];
	int port;
	SSL *ssl;
	enum ssl_handshake {SSL_HS_PENDING, SSL_HS_ERROR, SSL_HS_OK} ssl_hs_state;
};

struct server_t {
	int fd;
	int fd_un;
	int fd_sig;
	int fd_num;
	int max_lstn_fd;
	struct sockaddr_in addr;
	struct sockaddr_un addr_un;
	struct client_t *clients;
	size_t clients_size;
	SSL_CTX *ctx;
	int timeout;
	int write_pending_n;
};

void server_cleanup(struct server_t *srv);

int server_init(struct server_t *srv);

int server_main_loop(struct server_t *srv);

int server_accept(struct server_t *srv);

int server_read(struct client_t *clnt);

int server_send(struct client_t *clnt, const char *msg, size_t size);

int server_send_retry(struct client_t *clnt);

int server_remove_client(struct server_t *srv, int k);

int server_unix_read(struct server_t *srv);

enum ssl_handshake server_ssl_handshake(struct client_t *clnt);

#endif
