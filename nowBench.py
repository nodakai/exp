#! /usr/bin/env python

import sys
import time, datetime

def test01(n):
    for i in xrange(n):
        datetime.datetime.now()

def test02(n):
    nowFun = datetime.datetime.now
    for i in xrange(n):
        nowFun()

def test03(n):
    for i in xrange(n):
        time.time()

def test04(n, ts):
    for i in xrange(n):
        datetime.datetime.fromtimestamp(ts)

def bench(f, n, args=()):
    tb = datetime.datetime.now()
    f(n, *args)
    te = datetime.datetime.now()

    diff = te - tb
    us = diff.microseconds + 1e6 * diff.seconds
    print("%8.2f us => %6.4f us/call" % (us, us/n))

def main(args):
    n = 10 * 1000
    if len(args) > 0:
        n = int(args[0])

    bench(test01, n)
    bench(test02, n)
    bench(test03, n)
    bench(test04, n, args=(time.time(),))

if "__main__" == __name__:
    main(sys.argv[1:])
