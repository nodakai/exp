#! /usr/bin/env python

import sys
import struct

def main(args):
    fp = file(args[0])



    fmtA = "16sHHI"
    fmtB = "I6H"
    fmt32 = fmtA + "3P" + fmtB
    fmt64 = fmtA + "3Q" + fmtB

    res = struct.unpack(fmt64, fp.read(struct.calcsize(fmt64)))
    e_ident, e_type, e_machine, e_version, e_entry, e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx = res

    for v in ("e_ident", "e_type", "e_machine", "e_version", "e_entry", "e_phoff", "e_shoff", "e_flags", "e_ehsize", "e_phentsize", "e_phnum", "e_shentsize", "e_shnum", "e_shstrndx"):
        if v == "e_entry":
            print("%s == 0x%x" % (v, eval(v)))
        elif v == "e_ident":
            print("%s == %r" % (v, eval(v)))
        else:
            print("%s == %d" % (v, eval(v)))

if "__main__" == __name__:
    main(sys.argv[1: ])
