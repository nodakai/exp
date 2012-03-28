#! /usr/bin/python

import sys, os.path
import urllib
import subprocess

def main(args):
    fn = "primary.sqlite"
    p = subprocess.Popen(args=("sqlite3", fn), stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    sql = "SELECT location_href FROM packages WHERE name LIKE '%s' ;" % args[0]
    rpm_lst, _ = p.communicate(sql)
    rpm_lst = rpm_lst.strip().split("\n")
    if len(rpm_lst) < 1:
        sys.stderr.write("No such package.\n")
        sys.exit(20)
    elif len(rpm_lst) > 1:
        sys.stdout.write("%r\n" % rpm_lst)
        rpm = rpm_lst[int(args[1])]
    else:
        rpm = rpm_lst[0]
    ofn = os.path.basename(rpm)
    sys.stdout.write("%s => %s\n" % (sql, ofn))

    url_pfx = "http://ftp.jaist.ac.jp/pub/Linux/CentOS/6/os/x86_64/"
    url = os.path.join(url_pfx, rpm)
    urllib.urlretrieve(url, ofn)
    sys.stdout.write("%s:%d bytes written.\n" % (ofn, os.path.getsize(ofn)))

if "__main__" == __name__:
    main(sys.argv[1:])
