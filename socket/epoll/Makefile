CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lpthread

all: server client bench1 bench2

server: server.c shared.h
	$(CC) $(CFLAGS) server.c shared.h -o server

client: client.c shared.h
	$(CC) $(CFLAGS) client.c shared.h -o client

bench1: bench1.c shared.h
	$(CC) $(CFLAGS) bench1.c shared.h -o bench1 $(LDLIBS)

bench2: bench2.c shared.h
	$(CC) $(CFLAGS) bench2.c shared.h -o bench2

clean:
	rm -f server client bench1 bench2

