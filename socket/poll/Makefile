CC = gcc
CFLAGS = -std=c11 -O2 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lpthread

all: server client

server: server.c shared.c shared.h
	$(CC) $(CFLAGS) -o server server.c shared.c

client: client.c shared.c shared.h
	$(CC) $(CFLAGS) -o client client.c shared.c

clean:
	rm -f server client

