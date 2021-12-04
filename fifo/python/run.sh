#!/bin/bash

# Compila el programa C
gcc -Wall -Wextra -o client client.c

# Elimina los fifo si existieran
rm -f server.fifo client.fifo

# Crea dos fifos nuevos
mkfifo server.fifo client.fifo

# Lanza el servidor en background
./server.py &
# Lanza el cliente
./client

