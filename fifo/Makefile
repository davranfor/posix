CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs

all: reader writer

reader: reader.c
	$(CC) $(CFLAGS) reader.c -o reader

writer: writer.c
	$(CC) $(CFLAGS) writer.c -o writer

clean:
	rm -f reader writer

