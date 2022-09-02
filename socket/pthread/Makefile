CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lpthread

all: server client

server: server.c shared.c shared.h
	$(CC) $(CFLAGS) server.c shared.c -o server $(LDLIBS)

client: client.c shared.c shared.h
	$(CC) $(CFLAGS) client.c shared.c -o client

clean:
	rm -f server client

