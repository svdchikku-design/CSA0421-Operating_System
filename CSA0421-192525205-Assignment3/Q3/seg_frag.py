"""Q3: load and remove 20 segments of random size; measure external fragmentation (first fit, 1500 KB memory, seed 7)."""
import random
random.seed(7); MEM = 1500  # KB
holes = [(0, MEM)]           # (start, size)
placed = {}                  # id -> (start,size)
def alloc(i, size):
    for k, (s, h) in enumerate(holes):
        if h >= size:
            placed[i] = (s, size)
            if h == size: holes.pop(k)
            else: holes[k] = (s + size, h - size)
            return True
    return False
def free(i):
    s, size = placed.pop(i); holes.append((s, size)); holes.sort()
    m = []
    for a, b in holes:
        if m and m[-1][0] + m[-1][1] == a: m[-1] = (m[-1][0], m[-1][1] + b)
        else: m.append((a, b))
    holes[:] = m
sizes = [random.randint(20, 120) for _ in range(20)]
print("segment sizes (KB):", sizes, "total", sum(sizes))
for i, s in enumerate(sizes): assert alloc(i, s)
print(f"after loading 20 segments: used {sum(sizes)} KB, free {sum(h for _,h in holes)} KB in {len(holes)} hole(s)")
rem = random.sample(range(20), 10)
for i in rem: free(i)
fr = sorted(h for _, h in holes)
print(f"after removing 10 random segments {sorted(rem)}: holes (KB) = {[h for _,h in holes]}")
tf = sum(fr); big = fr[-1]
print(f"total free {tf} KB, largest hole {big} KB -> external fragmentation = 1 - largest/total = {100*(1-big/tf):.1f}%")
req = 450
print(f"a new segment of {req} KB: total free {tf} KB >= {req} KB but largest hole {big} KB -> {'FITS' if big>=req else 'CANNOT BE LOADED (external fragmentation)'}")
# compaction cost
moved = sum(sz for _, (s, sz) in placed.items())
print(f"compaction would copy {moved} KB of live segments (about {moved/ (10*1024)*1000:.1f} ms at 10 GB/s) to merge the holes")
# with paging 4 KB: internal fragmentation of same segments
ps = 4
waste = sum((-s) % ps for s in sizes)
print(f"same {len(sizes)} segments stored as 4 KB pages: internal fragmentation {waste} KB (<= {len(sizes)*3} KB), external fragmentation 0")
