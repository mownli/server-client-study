BINNAME = qwerty
INSTALLDIR = ~/.local/share/bin
CC = gcc
CFLAGS = -g -O2 -Wall
#CFLAGS = -g -fsanitize=address -O2 -Wall
LIBS = -lssl -lcrypto -pthread
SRCMODULES = server_init.c server_main_loop.c server_cleanup.c server_read.c server_accept.c server_remove_client.c server_send.c server_unix_read.c server_openssl.c server_ssl_handshake.c
OBJMODULES = $(SRCMODULES:.c=.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

main: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM $^ > $@

run: main
	./main

install: main
	install main $(INSTALLDIR)/$(BINNAME)

clean:
	rm -f *.o main deps.mk
