#!/usr/bin/env python3

import os, sys

try:
    server = os.open("server.fifo", os.O_RDONLY)
    client = os.open("client.fifo", os.O_WRONLY)
except Exception as err:
    raise SystemExit(err)

while True:
    data = os.read(server, 128).decode()
    if data == "":
        os.close(server)
        os.close(client)
        sys.exit()
    try:
        data = str(eval(data)) + "\n"
    except Exception:
        data = "Please, type a valid expression\n"
    os.write(client, data.encode())

