CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lrt -lpthread

all: reader writer

reader: reader.c shared.h
	$(CC) $(CFLAGS) reader.c -o reader $(LDLIBS)

writer: writer.c shared.h
	$(CC) $(CFLAGS) writer.c -o writer $(LDLIBS)

clean:
	rm -f reader writer

