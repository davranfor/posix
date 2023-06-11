#!/bin/bash

# Important: Run this script as ./client.sh not as sh client.sh
#            Otherwise redirection fails

while read line; do
    if [[ "$line" == 'quit' ]]; then
        break
    fi
    printf "$line\n\000"
done > >(netcat -q 1 127.0.0.1 8888)

for i in {1..25}; do
    sleep 0.0001
    printf "%02d) Hola desde netcat\n\000" $i
done > >(netcat -q 1 127.0.0.1 8888)

