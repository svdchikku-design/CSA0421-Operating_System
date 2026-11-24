"""Q7 Stage 3: page-replacement simulation of the edge device (page unit = 1 MB to keep numbers readable).
RAM 2048 MB, OS+kernel 200 MB -> 1848 MB for tasks. No swap: only file-backed (clean) pages can be evicted and re-read from flash.
Tasks: RT control (100 MB, LOCKED, touched every cycle), ML (1500 MB model, scanned sequentially once per inference),
logger (150 MB ring buffer, written sequentially), optional 4th task (500 MB, random access)."""
import random, collections, json
random.seed(3)
FRAMES = 2048 - 200
def trace(four):
    t = []; rt = [("rt", i) for i in range(100)]
    ml = [("ml", i) for i in range(1500)]; lg = 0; fo = 0
    for inf in range(8):                                   # 8 inferences
        for k in range(0, 1500, 15):
            t += [rt[(k // 15 * 10 + j) % 100] for j in range(10)]       # RT touches 10 pages per step
            t += ml[k:k + 15]
            t.append(("log", lg % 150)); lg += 1
            if four: t += [("t4", random.randrange(500)) for _ in range(5)]
    return t
def simulate(t, frames, policy):
    pinned = {p for p in set(t) if p[0] == "rt"}
    free = frames - len(pinned); res = collections.OrderedDict(); faults = collections.Counter(); hand = []; bits = {}
    nxt = None
    if policy == "opt":
        nxt = [0] * len(t); last = {}
        for i in range(len(t) - 1, -1, -1): nxt[i] = last.get(t[i], 10**9); last[t[i]] = i
        nu = {}
    q = collections.deque()
    for i, p in enumerate(t):
        if p in pinned:
            if ("rt_loaded", 0) not in faults: pass
            if p not in res: faults["rt"] += 1; res[p] = 1      # first touch of a locked page = loaded at start (mlock), counted below as start-up
            continue
        if p in res:
            if policy == "lru": res.move_to_end(p)
            if policy == "clock": bits[p] = 1
            if policy == "opt": nu[p] = nxt[i]
            continue
        faults[p[0]] += 1
        if len([x for x in res if x not in pinned]) >= free:
            if policy == "fifo": v = q.popleft()
            elif policy == "lru": v = next(x for x in res if x not in pinned)
            elif policy == "opt": v = max((x for x in res if x not in pinned), key=lambda x: nu[x])
            else:
                while True:
                    v = q.popleft()
                    if bits.get(v): bits[v] = 0; q.append(v)
                    else: break
            del res[v]; bits.pop(v, None)
        res[p] = 1; q.append(p); bits[p] = 1
        if policy == "opt": nu[p] = nxt[i]
    return faults
rows = []
for name, four, fr in (("3 tasks (design)", False, FRAMES), ("4th task +500 MB", True, FRAMES), ("4th task, ML model quantised to 1000 MB", "q", FRAMES)):
    if four == "q":
        random.seed(3); t = [p if p[0] != "ml" or p[1] < 1000 else None for p in trace(True)]; t = [p for p in t if p]
    else:
        random.seed(3); t = trace(four)
    pages = len(set(t)); print(f"\n{name}: {len(t)} references, {pages} MB touched (of {FRAMES} MB available), RT locked = 100 MB")
    print(f"{'policy':8s} {'RT faults after start':>22s} {'ML faults':>10s} {'logger':>7s} {'task4':>6s} {'total':>7s}   ML fault rate")
    for pol in ("fifo", "lru", "clock", "opt"):
        f = simulate(t, fr, pol); rtf = f["rt"] - 100; tot = sum(v for k, v in f.items() if k != "rt")
        mlrefs = sum(1 for p in t if p[0] == "ml")
        print(f"{pol:8s} {rtf:22d} {f['ml']:10d} {f['log']:7d} {f['t4']:6d} {tot:7d}   {100*f['ml']/mlrefs:5.1f}%")
        rows.append((name, pol, rtf, f["ml"], f["log"], f["t4"], tot))
json.dump(rows, open("edge_sim.json", "w"))
