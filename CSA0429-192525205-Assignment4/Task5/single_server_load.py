"""Task 5 alternative: one server shared by a growing number of users (real measurement on localhost) + queueing model."""
import threading, time, http.server, socketserver, urllib.request, statistics as S, os, sys
DATA = os.urandom(1 << 20)                                    # a 1 MB file
class H(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def do_GET(self):
        time.sleep(0.004)                                     # 4 ms of disk service time (assumption) per request, serialised below
        with LOCK: time.sleep(0.004)
        self.send_response(200); self.send_header("Content-Length", str(len(DATA))); self.end_headers(); self.wfile.write(DATA)
    def do_POST(self):
        n = int(self.headers["Content-Length"]); self.rfile.read(n)
        with LOCK: time.sleep(0.012)                          # a write must reach the disk: 12 ms (assumption), serialised
        self.send_response(200); self.send_header("Content-Length", "2"); self.end_headers(); self.wfile.write(b"ok")
    def log_message(self, *a): pass
LOCK = threading.Lock()
class TS(socketserver.ThreadingMixIn, http.server.HTTPServer): daemon_threads = True; request_queue_size = 256
srv = TS(("127.0.0.1", 0), H); port = srv.server_address[1]; threading.Thread(target=srv.serve_forever, daemon=True).start()
def client(kind, n, out):
    for _ in range(n):
        t = time.perf_counter()
        if kind == "read": urllib.request.urlopen(f"http://127.0.0.1:{port}/f").read()
        else: urllib.request.urlopen(urllib.request.Request(f"http://127.0.0.1:{port}/f", data=DATA)).read()
        out.append((time.perf_counter() - t) * 1000)
print("one server, every request needs the shared disk for 8 ms (read) or 12 ms (write): requests are served one after another (assumed disk times)")
print(f"{'users':>6} {'read: mean (ms)':>16} {'read: p95':>10} {'read req/s':>11} {'write: mean (ms)':>17} {'write req/s':>12}")
rows = []
for users in (1, 2, 4, 8, 16, 32, 64):
    res = {}
    for kind in ("read", "write"):
        out = []; ths = [threading.Thread(target=client, args=(kind, 15, out)) for _ in range(users)]
        t0 = time.perf_counter(); [t.start() for t in ths]; [t.join() for t in ths]; el = time.perf_counter() - t0
        res[kind] = (S.mean(out), sorted(out)[int(len(out) * .95) - 1], len(out) / el)
    rows.append((users, res)); print(f"{users:>6} {res['read'][0]:>16.1f} {res['read'][1]:>10.1f} {res['read'][2]:>11.0f} {res['write'][0]:>17.1f} {res['write'][2]:>12.0f}")
import json; json.dump([(u, r) for u, r in rows], open("single_server_load.json", "w"))
