#! /usr/bin/env python

from __future__ import print_function
import sys
import getopt
import datetime, time
import socket, select
import struct

def add_to_module(mod, name, val):
    if not getattr(mod, name, None):
        setattr(mod, name, val)

add_to_module(socket, 'ETH_P_ALL', 3)  # if you want ARP etc as well
add_to_module(socket, 'ETH_P_IP', 0x0800)

g_verbose = 0

# https://wiki.wireshark.org/Development/LibpcapFileFormat
PCAP_FILE_HDR = 'IHHiIII'
PCAP_REC_HDR = 'IIII'

IP_MREQN = '4s4si'
IP_MREQ_SOURCE = '4s4s4s'

n = datetime.datetime.now

def ns(now = n()):
    return now.strftime('%H:%M:%S.%f')

def capture(fp, interfaces, multicasts):
    try:
        SNAPLEN = 256 * 1024
        ETHERNET = 1
        fp.write(struct.pack(PCAP_FILE_HDR, 0xA1B2C3D4, 2, 4, 0, 0, SNAPLEN, ETHERNET))

        epoll = select.epoll()

        mc_fd_to_sock = {}
        for addr, port, local_iface, mc_src in multicasts:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, struct.pack('i', 1))
            s.bind((addr, port))
            try:
                if mc_src: # Source Specific Multicast
                    mreq_src = struct.pack(IP_MREQ_SOURCE, socket.inet_aton(addr), socket.inet_aton(local_iface), socket.inet_aton(mc_src))
                    s.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP_SOURCE, mreq_src)
                else:
                    mreqn = struct.pack(IP_MREQN, socket.inet_aton(addr), socket.inet_aton(local_iface), 0)
                    s.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, mreqn)
            except socket.error:
                print("arguments: {0!r}".format((addr, local_iface, mc_src)))
                raise

            mc_fd_to_sock[s.fileno()] = s
            epoll.register(s.fileno(), select.EPOLLIN)

        ps_fd_to_sock = {}
        for iface in interfaces:
            s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(socket.ETH_P_IP))
            s.bind((iface, 0))
            ps_fd_to_sock[s.fileno()] = s
            epoll.register(s.fileno(), select.EPOLLIN)

        MEGA = 10**6

        pkts_cnt = 0
        bytes_cnt = 0
        last_print = n()
        while True:
            try:
                for fd, evt in epoll.poll(1.):
                    if fd in mc_fd_to_sock:
                        _ = mc_fd_to_sock[fd].recv(1)
                    elif fd in ps_fd_to_sock:
                        dat = ps_fd_to_sock[fd].recv(9999999)
                        t = time.time()
                        datlen = len(dat)
                        snaplen = SNAPLEN if SNAPLEN < datlen else datlen
                        fp.write(struct.pack(PCAP_REC_HDR, int(t), int(MEGA * t) % MEGA, snaplen, datlen))
                        if SNAPLEN < datlen:
                            fp.write(dat[ :snaplen])
                        else:
                            fp.write(dat)
                        if g_verbose:
                            now = datetime.datetime.fromtimestamp(t)
                            print("{0}: {1}: {2!r}; {3} packets; {4} bytes".format(ns(now), datlen, dat, pkts_cnt, bytes_cnt))

                        pkts_cnt += 1
                        bytes_cnt += snaplen
                        if 0 == pkts_cnt % 10000:
                            now = datetime.datetime.fromtimestamp(t)
                            print("{0}: {1} packets; {2} bytes".format(ns(now), pkts_cnt, bytes_cnt))
                            last_print = now
            except socket.timeout:
                fp.flush()
            except KeyboardInterrupt:
                print("{0}: {1} packets; {2} bytes".format(ns(), pkts_cnt, bytes_cnt))
                return

            now = n()
            if 60. < (now - last_print).total_seconds():
                print("{0}: {1} packets; {2} bytes".format(ns(now), pkts_cnt, bytes_cnt))
                last_print = now

    except Exception:
        print("{0!r}".format(sys.exc_info()))
        raise

def parse_opts(args0):
    try:
        opts, args = getopt.getopt(args0, "i:o:v:a:p:l:s:")
        out_path = None
        interfaces = []
        addrs = []
        ports = []
        local_ifaces = []
        mc_sources = []
        for o, a in opts:
            if "-i" == o:
                interfaces.append(a)
            elif "-o" == o:
                out_path = a
            elif "-a" == o:
                addrs.append(a)
            elif "-p" == o:
                ports.append(int(a))
            elif "-l" == o:
                local_ifaces.append(a)
            elif "-s" == o:
                mc_sources.append(a)
            elif "-v" == o:
                global g_verbose
                g_verbose = int(a)

        assert 0 == len(args), "leftover: {0!r}".format(args)

        assert out_path, "output file must be given with -o"
        assert 0 < len(interfaces), "lisening interface must be given with -i"

        assert 0 < len(addrs)
        if 0 == len(local_ifaces):
            local_ifaces = [ '0.0.0.0' for _ in addrs ]
        if 0 == len(mc_sources):
            mc_sources = [ None for _ in addrs ]

        multicasts = []
        for i in range(len(addrs)):
            multicasts.append((addrs[i], ports[i], local_ifaces[i], mc_sources[i]))

        return out_path, interfaces, multicasts

    except Exception:
        print("invalid command line params: {0!r}".format(args0))
        print(sys.exc_info())
        raise

def main(args):
    print("{0}: started.".format(ns()))

    out_path, interfaces, multicasts = parse_opts(args)

    with open(out_path, 'wb') as fp:
        capture(fp, interfaces, multicasts)

    print("{0}: bye.".format(ns()))

if '__main__' == __name__:
    main(sys.argv[1: ])
