CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lpthread

all: server client stress

server: server.c shared.h
	$(CC) $(CFLAGS) server.c shared.h -o server

client: client.c shared.h
	$(CC) $(CFLAGS) client.c shared.h -o client

stress: stress.c shared.h
	$(CC) $(CFLAGS) stress.c shared.h -o stress $(LDLIBS)

clean:
	rm -f server client stress

