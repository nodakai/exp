#! /usr/bin/python

import sys, re, os, os.path
import xml.dom.minidom as dom
import urllib
import threading, Queue
import bz2

TYPE2STR = {}
for pfx in ("ELEMENT", "ATTRIBUTE", "TEXT", "CDATA_SECTION", "ENTITY",
        "PROCESSING_INSTRUCTION", "COMMENT", "DOCUMENT", "DOCUMENT_TYPE", "NOTATION"):
    n = pfx + "_NODE"
    TYPE2STR[getattr(dom.Node, n)] = n

def parse(elem, indent=0):
    if elem.nodeType == dom.Node.TEXT_NODE:
        s = elem.nodeValue.strip()
        if len(s) > 0:
            sys.stdout.write("  "*indent)
            sys.stdout.write("%r\n" % s)
    else:
        sys.stdout.write("  "*indent)
        sys.stdout.write("%s:%s" % (TYPE2STR[elem.nodeType], elem.localName))
        attrs = elem.attributes
        if attrs:
            for k,v in attrs.items():
                sys.stdout.write(" %s=%r" % (k, v))
        sys.stdout.write("\n")

        for cn in elem.childNodes:
            parse(cn, indent+1)

MAX_HTTP = 4
q = Queue.Queue(MAX_HTTP)

def dl(url):
    q.put(1, True)
    try:
        ifn = os.path.basename(url)
        urllib.urlretrieve(url, ifn)
        arch = bz2.BZ2File(ifn, buffering=10*1024**2)
        ofn = re.sub("^[^-]*-", "", os.path.splitext(ifn)[0])
        dec = file(ofn, "wb")
        sys.stdout.write("%s->%s\n" % (ifn, ofn))
        dec.write(arch.read())
        del dec
        sys.stdout.write("%s:%d bytes written.\n" % (ofn, os.path.getsize(ofn)))
        os.unlink(ifn)
    except:
        sys.stderr.write(sys.exc_info())
        q.get()

def main(args):
    top = dom.parse(file(args[0], "rb"))
    # parse(top)

    url_pfx = "http://ftp.jaist.ac.jp/pub/Linux/CentOS/5/os/x86_64/"

    threads = []

    for d in top.getElementsByTagName("data"):
        v = d.attributes["type"].nodeValue
        if re.search("_db$", v):
            sys.stdout.write("%s\n" % v)
            loc = d.getElementsByTagName("location")[0]
            v = loc.attributes["href"].nodeValue
            bn = os.path.basename(v)
            if os.path.isfile(bn) or os.path.isfile(re.sub("^[^-]*-", "", os.path.splitext(bn)[0])):
                sys.stdout.write("\tSkipping...\n")
            else:
                url = os.path.join(url_pfx, v)
                sys.stdout.write("\tGoing to D/L %s...\n" % url)
                t = threading.Thread(target=dl, args=(url,))
                t.start()
                threads.append(t)

    sys.stdout.write(("=" * 80) + "\n")
    sys.stdout.flush()
    for t in threads:
        t.join()

if "__main__" == __name__:
    main(sys.argv[1:])
