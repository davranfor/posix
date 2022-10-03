CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lpthread

all: server client

server: server.c shared.h
	$(CC) $(CFLAGS) server.c shared.h -o server

client: client.c shared.h
	$(CC) $(CFLAGS) client.c shared.h -o client $(LDLIBS)

clean:
	rm -f server client

