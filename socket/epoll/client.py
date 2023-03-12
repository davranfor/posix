import sys
import socket

HOST = '127.0.0.1'
PORT = 8888
BUFFER_SIZE = 32768
EOT = 0x04

def handle(sock):
    for data in sys.stdin:
        data = data + chr(EOT)
        try:
            sock.sendall(data.encode())
        except socket.error as e: 
            print('Error sending data: %s' % e, file = sys.stderr)
            sys.exit(1) 
        data = b''
        rcvd = 0
        while True:
            try:
                data += sock.recv(BUFFER_SIZE)
            except socket.error as e: 
                print('Error receiving data: %s' % e, file = sys.stderr)
                sys.exit(1)
            size = len(data)
            if size == rcvd:
                return
            if data[-1] == EOT:
                data = data.decode()[:-1]
                break
            rcvd = size
        print('Size: ' + str(len(data)) + ' | Server says: ' + data, end = '')

def main():
    address = (HOST, PORT)
    print('Connecting to %s port %s' % address)
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(address)
    except socket.error as e: 
        print('Error connecting to server: %s' % e, file = sys.stderr)
        sys.exit(1) 
    handle(sock)
    print('Client exits')
    sock.close()

if __name__ == '__main__':
    main()

