#!/usr/bin/python3

import os, sys

fd = int(sys.argv[1])
n = 128

while True:
    buf = os.read(fd, n)
    if buf.decode() == "quit":
        os.close(fd)
        sys.exit()
    else:
        print("parent says '" + buf.decode() + "'")
        os.write(fd, buf)

