#!/usr/bin/python3

import os, sys

fd = int(sys.argv[1])
n = 128

while True:
    buf = os.read(fd, n)
    print("Parent sent " + buf.decode())
    if buf.decode() == "quit":
        os.close(fd)
        print("Child says Bye!")
        sys.exit()
    else:
        os.write(fd, buf)

