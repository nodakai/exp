#! /usr/bin/env python

"""A tcpdump(8)-compatible pcap file reader.
Note that this program doesn't have an ability to capture packets.
"""

import sys
import struct
import optparse
import datetime

global g_debug
g_debug = False

class pytcpdump(object):
    def __init__(self, opts, args):
        fn = args[0]
        self.fp = open(fn)

        gfmt = "IHHiIII"
        self.gfmt = (gfmt, struct.calcsize(gfmt))
        pfmt = "IIII"
        self.pfmt = (pfmt, struct.calcsize(pfmt))

        self.COUNT = opts.count

        self.nbytes = 0

    def do(self):
        rawdat = self.fp.read(self.gfmt[1])
        if len(rawdat) < self.gfmt[1]:
            raise InvalidArgument("Couldn't read a full pcap header; only %d bytes read" % len(rawdat))

        magic_number, version_major, version_minor, thiszone, sigfigs, snaplen, network \
            = struct.unpack(self.gfmt[0], rawdat)
        if g_debug:
            print "magic=%08x; version=%d.%d; zone=%d; figs=%d; snaplen=%d; network=%d" % \
                (magic_number, version_major, version_minor, thiszone, sigfigs, snaplen, network)

        cnt = 0
        try:
            while True:
                self.parse_packet()
                cnt += 1
                if self.COUNT > 0 and cnt > self.COUNT:
                    return 0
        except StopIteration:
            print "%d packets read." % cnt

        return 0

    def parse_packet(self):
        rawdat = self.fp.read(self.pfmt[1])
        if len(rawdat) < self.pfmt[1]:
            raise StopIteration
        self.nbytes += len(rawdat)

        ts_sec, ts_usec, incl_len, orig_len = struct.unpack(self.pfmt[0], rawdat)
        secs = ts_sec + 1e-6 * ts_usec
        tm = datetime.datetime.fromtimestamp(secs)
        rawdat = self.fp.read(incl_len)
        if len(rawdat) < incl_len:
            raise StopIteration

        if g_debug:
            print "%s %d" % (tm, len(rawdat))
        self.nbytes += len(rawdat)

def show_nbytes(nbytes):
    units = ("KB", "MB", "GB", "TB") # TB??
    units_sel = 0
    kiB = 1024.0
    while nbytes > kiB:
        nbytes /= kiB
        units_sel += 1
    if units_sel == 0:
        human_friendly = ""
    else:
        human_friendly = "(%.1f %s) " % (nbytes, units[units_sel - 1])
    print "%d bytes %sread" % (ptd.nbytes, human_friendly)

if __name__ == '__main__':
    opt_parser =  optparse.OptionParser()
    opt_parser.add_option("-d", "--debug", action="store", type="int", dest="debug",
        default=0, help="Debug output level")
    opt_parser.add_option("-c", "--count", action="store", type="int", dest="count",
        default=0, help="max # of packets analyzed")
    opts, args = opt_parser.parse_args(sys.argv[1:])
    g_debug = opts.debug

    ptd = pytcpdump(opts, args)
    rc = ptd.do()
    show_nbytes(ptd.nbytes)
    sys.exit(rc)
