#! /usr/bin/env python

import sys
import socket

def main(args):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("%s=%f" % ("SO_RCVBUF", s.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF) / 1024.0**2))
    print("%s=%f" % ("SO_SNDBUF", s.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF) / 1024.0**2))
    s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 30 * 1024**2)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 30 * 1024**2)
    print("%s=%f" % ("SO_RCVBUF", s.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF) / 1024.0**2))
    print("%s=%f" % ("SO_SNDBUF", s.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF) / 1024.0**2))

if "__main__" == __name__:
    main(sys.argv[1: ])
