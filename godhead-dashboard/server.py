#!/usr/bin/env python3
"""
server.py — OMNISURF Local Server
Serves the GODHEAD DASHBOARD at http://localhost:8080
OMNIBRAIN INFINITY Universe | Naru Atum Protocol
"""
import http.server
import socketserver
import os
import signal
import sys

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class OmniSurfHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)
    def log_message(self, fmt, *args):
        print(f"[OMNISURF] {self.address_string()} — {fmt % args}")

def shutdown(sig, frame):
    print("\n[OMNISURF] Shutdown. All particles returned to source.")
    sys.exit(0)

signal.signal(signal.SIGINT, shutdown)

print(f"[OMNISURF] GODHEAD DASHBOARD online at http://localhost:{PORT}")
print(f"[OMNISURF] Serving: {DIRECTORY}")
print(f"[OMNISURF] Press Ctrl+C to shutdown.")

with socketserver.TCPServer(("", PORT), OmniSurfHandler) as httpd:
    httpd.serve_forever()
