from sys import stdin
import socket

EOT = 0x04

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('127.0.0.1', 8888)
print('connecting to %s port %s' % server_address)
sock.connect(server_address)

try:
    for message in stdin:
        message = message + chr(EOT)
        sock.sendall(message.encode())

        data = []
        while True:
            chunk = sock.recv(32768)
            data.append(chunk)
            if chunk[-1] == EOT:
                # b''.join to caoncat all elements of the list
                # [:-1] removes the last character, in this case EOT
                data = b''.join(data).decode()[:-1]
                break
        print('Size: ' + str(len(data)) + ' | Server says: ' + data, end = '')
finally:
    print('Client exits')
    sock.close()
