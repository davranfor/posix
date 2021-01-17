CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs

all: main.c
	$(CC) $(CFLAGS) main.c -o decode

clean:
	rm -f decode
