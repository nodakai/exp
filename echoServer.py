#! /usr/bin/env python

import sys
import time, datetime
import threading
import socket, select

class Server(threading.Thread):
    def __init__(self, c_sock):
        threading.Thread.__init__(self)
        self.c_sock = c_sock

    def run(self):
        try:
            while True:
                dat = self.c_sock.recv(2**16)
                if 0 == len(dat) or "exit" == dat.rstrip():
                    return
                sys.stdout.write("<<%4d<<%r<<\n" % (len(dat), dat))
                sys.stdout.flush()
                self.c_sock.send(dat)
        finally:
            self.c_sock.close()
            self.c_sock = None

def main(args):
    addr, port = args[ :2]
    port = int(port)

    if len(args) > 1:
        addr = args[0]
        port = int(args[1])
    else:
        addr = "0.0.0.0"
        port = int(args[0])

    s_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s_sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    s_sock.bind((addr, port))
    s_sock.listen(10)

    servers = []
    while True:
        c_sock, addr = s_sock.accept()
        s = Server(c_sock)
        s.start()

    for s in servers:
        s.join()


if "__main__" == __name__:
    main(sys.argv[1: ])
