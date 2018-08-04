#!/usr/bin/env python2

import BaseHTTPServer, SimpleHTTPServer
import ssl

httpd = BaseHTTPServer.HTTPServer(('0', 443), SimpleHTTPServer.SimpleHTTPRequestHandler)
httpd.socket = ssl.wrap_socket(httpd.socket, certfile='/etc/letsencrypt/live/2718.ml/fullchain.pem',
                               keyfile='/etc/letsencrypt/live/2718.ml/privkey.pem',
                               server_side=True)
httpd.serve_forever()
