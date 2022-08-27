CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lpthread

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LDLIBS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client

