CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -O2

all: server client

server: server.c shared.c shared.h buffer.c buffer.h
	$(CC) $(CFLAGS) server.c shared.c buffer.c -o server

client: client.c shared.c shared.h buffer.c buffer.h
	$(CC) $(CFLAGS) client.c shared.c buffer.c -o client -lpthread

clean:
	rm -f server client

