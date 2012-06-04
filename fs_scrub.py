#! /usr/bin/env python

import sys
import os, glob
import time

def walk(path, top=True):
    for p in glob.glob(os.path.join(path, "*")):
        print(p)
        if p not in (".", ".."): # os.path.basename(p)[0] != ".":
            if os.path.isdir(p):
                walk(p)
            else:
                _ = file(p).read()

def main(args):
    if len(args):
        path = args[0]
    else:
        path = "."
    # walk(path)

    t0 = time.time()
    acc = []
    for the_dir, sub_dirs, files in os.walk(path):
        for f in files:
            acc.append(os.path.join(the_dir, f))
            # print(f)

    for i in xrange(5):
        sys.stdout.write("%-15d" % (i*100/5))
    sys.stdout.write("100%\n")
    tot = float(len(acc))
    STEP = 1.0 / 80
    thresh = STEP
    for i, f in enumerate(acc):
        while i / tot > thresh:
            sys.stdout.write("-")
            sys.stdout.flush()
            thresh += STEP
        fp = file(f)
        while True:
            if (len(fp.read(2**30)) < 2**30):
                break
    sys.stdout.write("\n")
    t1 = time.time()
    sys.stdout.write("Took %.1f sec.\n" % (t1 - t0))


if "__main__" == __name__:
    main(sys.argv[1: ])
