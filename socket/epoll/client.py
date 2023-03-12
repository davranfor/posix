import sys
import socket

HOST = '127.0.0.1'
PORT = 8888
BUFFER_SIZE = 32768
EOT = 0x04

def handle(sock):
    for message in sys.stdin:
        message = message + chr(EOT)

        try:
            sock.sendall(message.encode())
        except socket.error as e: 
            print('Error sending data: %s' % e, file = sys.stderr)
            sys.exit(1) 

        data = []
        while True:
            try:
                chunk = sock.recv(BUFFER_SIZE)
            except socket.error as e: 
                print('Error receiving data: %s' % e, file = sys.stderr)
                sys.exit(1) 
            if not chunk:
                return
            data.append(chunk)
            if chunk[-1] == EOT:
                # b''.join to concat all elements of the list
                # [:-1] removes the last character, in this case EOT
                data = b''.join(data).decode()[:-1]
                break
        print('Size: ' + str(len(data)) + ' | Server says: ' + data, end = '')

addr = (HOST, PORT)
print('Connecting to %s port %s' % addr)
try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(addr)
except socket.error as e: 
    print('Error connecting to server: %s' % e, file = sys.stderr)
    sys.exit(1) 
handle(sock)
print('Client exits')
sock.close()

