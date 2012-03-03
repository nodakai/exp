#! /usr/bin/env python

import sys

i = 0
while True:
    line = sys.stdin.readline()
    if not line:
        break
    if i == 0:
        k = float(line.rstrip())
    elif i <= 2:
        a = []
        b = []
    elif i <= 7:
        a.append(float(line.split()[2]))
    else:
        b.append(float(line.split()[2]))

    if i == 12:
        a.sort()
        b.sort()
        print "%f, %f, %f" % (k, 1e-6 * a[2], 1e-6 * b[2])

    i = (i+1) % 13

