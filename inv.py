#! /usr/bin/env python

import sys

def main(args):
    aum = float(args[0])
    year_expense = float(args[1])
    mon_rate = float(args[2])
    years = int(args[3])

    for y in xrange(1, 1+years):
        v = aum
        for m in xrange(12):
            v -= year_expense / 12
            v *= mon_rate
        profit = 0.7 * (v - aum)
        aum += profit

        sys.stdout.write("{} : {:,.0f} : {:,.0f}\n".format(y, aum, profit))

if "__main__" == __name__:
    main(sys.argv[1: ])
