#!/bin/bash

gcc -Wall -Wextra -o client client.c

rm -f server.fifo client.fifo

mkfifo server.fifo client.fifo

./server.py &
./client

