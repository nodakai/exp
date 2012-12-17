#! /usr/bin/env python

import sys
import getopt
import datetime, time
import struct
import socket, select

LONG_ENOUGH = 2**20

ADDR_FMT = "IH"

def ip_int(ip_str):
    words = ip_str.split(".")
    return (int(words[0]) << 24) | (int(words[1]) << 16) | (int(words[2]) << 8) | int(words[3])

def pack(host, port, dat):
    pass

def do_server(port, host_tr_spec):
    host_tr = { }
    for src, dst in host_tr_spec:
        host_tr[ ip_int(src) ] = dst

    def unpack(dat):
        hdr_len = struct.calcsize(ADDR_FMT)
        host_int, port = struct.unpack(ADDR_FMT, dat[ :hdr_len])
        return (host_int, port, dat[hdr_len: ])

    svr_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    svr_sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    svr_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    svr_sock.bind(("localhost", port))
    svr_sock.listen(1)
    conn, addr = svr_sock.accept()
    print("The tunnel conn with {} was established.".format(addr))

    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock.bind(("0.0.0.0", 223))
    fd2s = { }

    epoll = select.epoll()
    conn_fd = conn.fileno()
    epoll.register(conn_fd, select.EPOLLIN)

    fd = udp_sock.fileno()
    epoll.register(fd, select.EPOLLIN)
    fd2s[fd] = udp_sock

    while True:
        evts = epoll.poll(10)
        for fd, evt in evts:
            if evt & select.EPOLLIN:
                if fd in fd2s:
                    s = fd2s[fd]
                    dat = s.recv(LONG_ENOUGH)
                    conn.send(dat)
                elif fd == conn_fd:
                    dat = conn.recv(LONG_ENOUGH)
#                       host_int, port, dat = unpack(dat)
#                   host = host_tr[host_int]
                    udp_sock.sendto(dat, ("127.0.0.1", 123))
#                   udp_sock.sendto(dat, ("202.150.213.154", 123))
                else:
                    print("??? {}".format((fd, evt)))
            elif evt & (select.EPOLLHUP | select.EPOLLERR):
                if fd == conn_fd:
                    sys.stderr.write("{} TCP tunnel was terminated.".format(
                        datetime.datetime.now().strftime("%H:%M:%S.%f")))
                    epoll.close()
                    tcp.close()
                    for fd, s in fd2s.iteritems():
                        s.close()
                    return
                else:
                    print("??? {}".format((fd, evt)))
        time.sleep(1e-2)

def do_client(tcp_port, bind_specs):
    tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    tcp.connect(("localhost", tcp_port))
    tcp_fd = tcp.fileno()

    fd2s = { }
    da2s = { }
    epoll = select.epoll()
    epoll.register(tcp_fd, select.EPOLLIN)
    for bs in bind_specs:
        saddr, port, daddr = bs.split(",")
        port = int(port)
        daddr = ip_int(daddr)
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind((saddr, port))
        fd = s.fileno()
        epoll.register(fd, select.EPOLLIN)
        fd2s[fd] = (s, port, daddr)
        da2s[daddr] = s
        tmp_s = s

    while True:
        evts = epoll.poll(10)
        for fd, evt in evts:
            if evt & select.EPOLLIN:
                if fd in fd2s:
                    s, port, daddr = fd2s[fd]
                    dat, a = s.recvfrom(LONG_ENOUGH)
                    sa, sp = a
                    tcp.send(dat)
#                   tcp.send(struct.pack("Is", daddr, dat))
                elif tcp_fd == fd:
                    dat = tcp.recv(LONG_ENOUGH)
#                   daddr, dat = struct.unpack("Is", dat)
#                   s = da2s[daddr]
                    tmp_s.send(dat)
                else:
                    print("??? {}".format((fd, evt)))
            elif evt & (select.EPOLLHUP | select.EPOLLERR):
                if fd in fd2s:
                    print("??? {}".format((fd, evt)))
                elif tcp_fd == fd:
                    sys.stderr.write("{} TCP tunnel was terminated.".format(
                        datetime.datetime.now().strftime("%H:%M:%S.%f")))
                    epoll.close()
                    tcp.close()
                    for fd, s in fd2s.iteritems():
                        s.close()
                    return
                else:
                    raise RuntimeError(("??? {}".format((fd, evt))))


def main(args):
    opts, args = getopt.getopt(args, "S:C:")
    for o, a in opts:
        if "-S" == o:
            do_server(int(a), args)
        elif "-C" == o:
            do_client(int(a), args)

    sys.stderr.write("{!r}\n".format((opts, args)))

if "__main__" == __name__:
    main(sys.argv[1: ])
