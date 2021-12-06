#!/bin/bash

# Compila el programa C
gcc -Wall -Wextra -o client client.c

# Compila el programa Java
javac Server.java

# Elimina los fifo si existieran
rm -f server.fifo client.fifo

# Crea dos fifos nuevos
mkfifo server.fifo client.fifo

# Lanza el servidor en background
java Server &
# Lanza el cliente
./client

