#!/usr/bin/env python3

import posix, sys, os

try:
    server = posix.open("server.fifo", os.O_RDONLY)
    client = posix.open("client.fifo", os.O_WRONLY)
except Exception as err:
    raise SystemExit(err)

while True:
    data = posix.read(server, 128).decode()
    if data == "":
        posix.close(server)
        posix.close(client)
        sys.exit()
    try:
        data = eval(data)
        data = str(data) + "\n"
    except Exception:
        data = "Please, type a valid expression\n"
    posix.write(client, data.encode())

