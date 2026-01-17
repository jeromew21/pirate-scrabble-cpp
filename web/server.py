#!/usr/bin/env python3
from http.server import HTTPServer, SimpleHTTPRequestHandler
import webbrowser
import threading

PORT = 8000

def open_browser():
    webbrowser.open(f"http://localhost:{PORT}/build-web/pirate-scrabble.html")

class CORSRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

if __name__ == '__main__':
    httpd = HTTPServer(('localhost', PORT), CORSRequestHandler)
    print(f"Server running on http://localhost:{PORT}")
    threading.Timer(1.0, open_browser).start()
    httpd.serve_forever()
