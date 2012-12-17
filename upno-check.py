#! /usr/bin/env python

import sys
import types
import re
import struct
import datetime

NO_RE = re.compile(r"NO *(\d+) *(\d+) *(\d+)")
TIME_RE_Q = re.compile(r"(QB|QS|BC|SC).{19}(\d{9})")
TIME_RE_F = re.compile(r"1P.{18}(\d{9})")

UNDEF_DATE = "(cannot read date/time)"

class UpNo:
    def __init__(self, no, sub_no, max_sub_no, date):
        self.no = int(no)
        self.sub_no = int(sub_no)
        self.max_sub_no = int(max_sub_no)
        try:
            if UNDEF_DATE == date:
                self.date = date
            else:
                self.date = "{}:{}:{}.{}".format(*struct.unpack("2s2s2s3s", date))
        except Exception:
            print date
            raise

    def last(self):
        return self.sub_no == self.max_sub_no

    def __str__(self):
        if 1 == self.max_sub_no:
            return "[NO {:6d} @ {}]".format(self.no, self.date)
        else:
            return "[NO {:6d},{:2d}/{:2d} @ {}]".format(self.no, self.sub_no, self.max_sub_no, self.date)

def process_generic(generator):
    err_cnt = 0
    prev_nos = { }
    for dat in generator:
        if isinstance(dat, types.TupleType):
            dat = dat[0][ dat[1]: ]

        m = NO_RE.search(dat)
        if not m: continue

        ch = int(dat[35:40])
        dm = TIME_RE_Q.search(dat)
        dt = UNDEF_DATE
        if dm:
            dt = dm.group(2)
        else:
            dm = TIME_RE_F.search(dat)
            if dm:
                dt = dm.group(1)
        this_no = UpNo(m.group(1), m.group(2), m.group(3), dt)

        prev_no = prev_nos.get(ch, None)
        if prev_no:
            global g_stat
            if prev_no.last() and prev_no.no + 1 == this_no.no and 1 == this_no.sub_no:
                g_stat[0] += 1
            elif not prev_no.last() and prev_no.no == this_no.no and prev_no.sub_no + 1 == this_no.sub_no:
                g_stat[0] += 1
            else:
                estim_loss = this_no.no - prev_no.no - 1
                if 0 == estim_loss:
                    estim_loss = this_no.sub_no - prev_no.sub_no - 1
                g_stat[0] += estim_loss
                g_stat[1] += estim_loss
                if 1 or err_cnt < 100:
                    print("Gap for {}: {} {} (Lost #update == {})".format(ch, prev_no, this_no, estim_loss))
                    err_cnt += 1
        prev_nos[ch] = this_no
    # print("The most recent NO was {} for {}.  Quitting...".format(prev_no, ch))
    for k,v in prev_nos.iteritems():
        print("{:<5d}: {}".format(k, v))

def process_sample_subscriber_output(fn):
    process_generic(open(fn))

def fmt_read(fp, fmt):
    return struct.unpack(fmt, fp.read(struct.calcsize(fmt)))

class EtherHdr(object):
    def __init__(self, dat):
        self.dat = dat

    def dst(self):
        d = self.dat[ :6]
        return ":".join(["{:02x}".format(ord(i)) for i in d])

    def src(self):
        d = self.dat[6:12]
        return ":".join(["{:02x}".format(ord(i)) for i in d])

    def typ(self):
        d = self.dat[12:14]
        return struct.unpack(">H", d)[0]

    def get_ip(self):
        if 0x800 == self.typ():
            return IpHdr(self.dat, 14)
        else:
            return None

    def __str__(self):
        return "{}->{} 0x{:04x}".format(self.src(), self.dst(), self.typ())

class IpHdr(object):
    def __init__(self, dat, off):
        self.dat = dat
        self.off = off

    @staticmethod
    def to_int32(d):
        ret = 0
        for i in xrange(4):
            ret |= (ord(d[3 - i]) << (8*i))
        return ret

    def src(self):
        o = self.off
        return IpHdr.to_int32( self.dat[ o+12 : o+12+4 ] )

    def src_str(self):
        o = self.off
        d = self.dat[ o+12 : o+12+4 ]
        return ".".join("{}".format(ord(i)) for i in d)

    def dst(self):
        o = self.off
        return IpHdr.to_int32( self.dat[ o+16 : o+16+4 ] )

    def dst_str(self):
        o = self.off
        d = self.dat[ o+16 : o+16+4 ]
        return ".".join("{}".format(ord(i)) for i in d)

    def proto(self):
        o = self.off
        return ord(self.dat[ o+9 : o+10 ])

    def hdr_len(self):
        return 4 * (ord(self.dat[self.off]) & 0xF)

    def get_udp(self):
        if 17 == self.proto():
            return UdpHdr(self.dat, self.off + self.hdr_len())
        else:
            return None

    def get_tcp(self):
        if 6 == self.proto():
            return TcpHdr(self.dat, self.off + self.hdr_len())
        else:
            return None

    def __str__(self):
        return "{}->{} {}".format(self.src_str(), self.dst_str(), self.proto())

class UdpHdr(object):
    def __init__(self, dat, off):
        self.dat = dat
        self.off = off

    def src(self):
        o = self.off
        return struct.unpack(">H", self.dat[ o : o+2 ])[0]

    def dst(self):
        o = self.off
        return struct.unpack(">H", self.dat[ o+2 : o+2+2 ])[0]

    def get_off(self):
        return (self.dat, self.off + 8)

    def __str__(self):
        return "{}->{}".format(self.src(), self.dst())

class TcpHdr(object):
    def __init__(self, dat, off):
        self.dat = dat
        self.off = off

    def src(self):
        o = self.off
        return struct.unpack(">H", self.dat[ o : o+2 ])[0]

    def dst(self):
        o = self.off
        return struct.unpack(">H", self.dat[ o+2 : o+2+2 ])[0]

    def get_off(self):
        d = self.dat
        o = self.off
        return (d, o + 4 * (ord(d[o + 12]) >> 4))

    def __str__(self):
        return "{}->{}".format(self.src(), self.dst())


def process_pcap(fn):

    def generator(fn):
        cnt = 0

        with open(fn) as fp:
            tpl = fmt_read(fp, "IHHiIII")
            magic, ver_maj, ver_min, tz, sig, snaplen, nw = tpl
            if 0xA1B2C3D4 != magic:
                raise RuntimeError("0x{:08X}, {}.{}, {}, {}, {}, {}".format(*tpl))

            try:
                while True:
                    # buf = []
                    sec, usec, incl, orig = fmt_read(fp, "4I")
#                   tm = datetime.datetime.fromtimestamp(sec + 1e-6 * usec)
                    dat = fp.read(incl)
                    if not len(dat): return
                    eth = EtherHdr(dat)
                    # buf.append(str(tm))
                    # buf.append(" {}".format(eth))

                    ip = eth.get_ip()
                    if not ip: continue
                    # if not (90 == (ip.dst() & 0xff)): continue

                    # buf.append(" {}".format(ip))

#                   tcp = ip.get_tcp()
#                   if not tcp: continue
#                   d, o = tcp.get_off()
#                   if len(d) <= o: continue
#                   yield (tm, d[o: ])

                    udp = ip.get_udp()
                    if not udp: continue
                    udp_dst = udp.dst()
                    if not (52031 <= udp_dst and udp_dst <= 52090): continue
#                   if 52051 != udp_dst: continue
                    tm = datetime.datetime.fromtimestamp(sec + 1e-6 * usec)
                    yield (tm, udp_dst, udp.get_off())

                    # buf.append(" {}".format(udp))

                    # sys.stdout.write("".join(buf) + "\n")
                    # cnt += 1
            except struct.error: pass

    # process_generic(generator(fn))
    prevs = {}
    err_cnt = 0
    for tm, port, d_o in generator(fn):
        if port in prevs:
            prev = prevs[port]
        else:
            prev = [0, None, None]
            prevs[port] = prev
        d, o = d_o
        try:
            this_seq_no = int(d[o+10: o+18])
            if prev[0]+1 != this_seq_no and err_cnt < 100:
                p_seq_no = prev[0]
                p_tm = prev[1]
                p_d, p_o = prev[2]
                estim_loss = this_seq_no - p_seq_no - 1
                print("{}: [{:8} @ {}] [{:8} @ {}] #gap=={}".format(port % 100, p_seq_no, p_tm.strftime("%H-%M-%S.%f"),
                    this_seq_no, tm.strftime("%H-%M-%S.%f"), estim_loss))
                err_cnt += 1
                g_stat[1] += estim_loss
            g_stat[0] += 1
            prev[0] = this_seq_no
            prev[1] = tm
            prev[2] = d_o
        except Exception:
            print(repr(d[o: ]))
            raise

def main(args):
    global g_stat
    g_stat = [0, 0]

    RE_PCAP = re.compile(r"\.pcap\Z", re.I)
    for a in args:
        if RE_PCAP.search(a):
            process_pcap(a)
        else:
            process_sample_subscriber_output(a)
    print("lost / tot == {} / {}".format(g_stat[1], g_stat[0]))

if '__main__' == __name__:
    main(sys.argv[1: ])
