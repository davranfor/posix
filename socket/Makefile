CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -O2
LDLIBS = -lpthread

all: server client

server: server.c shared.c shared.h pool.c pool.h
	$(CC) $(CFLAGS) server.c shared.c pool.c -o server $(LDLIBS)

client: client.c shared.c shared.h pool.c pool.h
	$(CC) $(CFLAGS) client.c shared.c pool.c -o client $(LDLIBS)

clean:
	rm -f server client

