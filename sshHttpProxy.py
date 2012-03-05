#! /usr/bin/env python

import sys
import socket
import select
import SocketServer

g_debug = False

class Handler(SocketServer.BaseRequestHandler):
    SSH = "SSH"
    HTTP = "HTTP"

    def handle(self):
        global g_debug

        self.proto = None
        while True:
            if self.proto:
                rs, _, _ = select.select([self.request, self.server.fwsock], [], [])
                if g_debug: print repr(rs)
                for s in rs:
                    if s == self.request:
                        dat = self.request.recv(2**16)
                        if len(dat) == 0:
                            print "closed"
                            self.server.fwsock.shutdown(socket.SHUT_WR)
                            return
                        if g_debug: print ">>%4d>>%r>>" % (len(dat), dat)
                        self.server.fwsock.send(dat)
                    elif s == self.server.fwsock:
                        dat = self.server.fwsock.recv(2**16)
                        if len(dat) == 0:
                            print "closed"
                            self.request.shutdown(socket.SHUT_WR)
                            return
                        if g_debug: print "<<%4d<<%r<<" % (len(dat), dat)
                        self.request.send(dat)
            else:
                rs, _, _ = select.select([self.request], [], [], 0.5)
                if len(rs) > 0:
                    dat = self.request.recv(2**16)
                    if len(dat) > 7 and dat[ :7] == "SSH-2.0":
                        self.proto = Handler.SSH
                        spec = self.server.ssh_spec
                    else:
                        self.proto = Handler.HTTP
                        spec = self.server.http_spec
                    self.server.fwsock.connect(spec)
                    self.server.fwsock.send(dat)
                else:
                    self.proto = Handler.SSH
                    spec = self.server.ssh_spec
                    self.server.fwsock.connect(spec)
                if g_debug: print repr(spec)

    def finish(self):
        self.server.server_close()

class ForkingTCPProxy(SocketServer.ForkingTCPServer):
    allow_reuse_address = True

    def __init__(self, server_address, ssh_spec, http_spec, RequestHandlerClass):
        SocketServer.ForkingTCPServer.__init__(self, server_address, RequestHandlerClass)
        self.ssh_spec = ssh_spec
        self.http_spec = http_spec
        self.fwsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def close_request(self, request):
        SocketServer.ForkingTCPServer.close_request(self, request)
        self.fwsock.close()

    def get_request(self):
        acpt = self.socket.accept()
        if g_debug: print repr(acpt)
        self.fwsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        return acpt


def main(args):
    global g_debug
    for o in args:
        if o == "-d":
            g_debug = True
    svr = ForkingTCPProxy((args[0], int(args[1])), (args[2], int(args[3])), (args[4], int(args[5])), Handler)
    if g_debug: print args
    svr.serve_forever()


if __name__ == "__main__":
    main(sys.argv[1:])
