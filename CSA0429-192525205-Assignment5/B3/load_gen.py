"""Closed-loop HTTP load generator (keep-alive). usage: load_gen.py PORT CLIENTS SECONDS"""
import sys, time, threading, http.client, random
port, clients, secs = int(sys.argv[1]), int(sys.argv[2]), float(sys.argv[3])
done = [0] * clients; errs = [0] * clients; lat = [[] for _ in range(clients)]
def worker(i):
    c = http.client.HTTPConnection("127.0.0.1", port); end = time.time() + secs
    while time.time() < end:
        t = time.perf_counter()
        try:
            c.request("GET", f"/result/{192525000 + random.randrange(5000)}"); r = c.getresponse(); r.read()
            if r.status != 200: errs[i] += 1
        except Exception: errs[i] += 1; c = http.client.HTTPConnection("127.0.0.1", port)
        lat[i].append((time.perf_counter() - t) * 1000); done[i] += 1
ts = [threading.Thread(target=worker, args=(i,)) for i in range(clients)]
t0 = time.time(); [t.start() for t in ts]; [t.join() for t in ts]; el = time.time() - t0
al = sorted(x for l in lat for x in l); n = sum(done)
print(f"requests={n} errors={sum(errs)} rps={n/el:.0f} mean={sum(al)/len(al):.1f}ms p95={al[int(len(al)*.95)]:.1f}ms p99={al[int(len(al)*.99)]:.1f}ms")
