#! /usr/bin/env python

import sys
import timeit
import random

def bench(ntimes):
    timeit.Timer()

def main(args):
    tmr = timeit.Timer("random.randint(0,9)", "import random; random.seed(1000)")
    try:
        print "%.2f us/pass" % tmr.timeit()
    except Exception:
        tmr.print_exc()

if __name__ == "__main__":
    main(sys.argv[1:])
