"""Q2 calculations: page-table size, effective access time, internal fragmentation of 100 random processes."""
import random
KB, MB = 1024, 1024**2
print("=== Page-table size, 4 KB pages, flat single-level table ===")
for bits, entry in ((32, 4), (48, 8)):
    n = 2**bits // (4 * KB)
    size = n * entry
    print(f"{bits}-bit address space: {n:,} pages x {entry} B entries = {size/MB:,.0f} MB per process"
          + (f" ({size/1024**3:,.0f} GB)" if size >= 1024**3 else ""))
# multi-level 48-bit: 4 levels of 9 bits, only used parts exist
print("48-bit, 4-level table (x86-64): each table = 512 entries x 8 B = 4 KB; a process using 16 MB (4096 pages) needs:")
pages = 16 * MB // 4096; l1 = -(-pages // 512); l2 = -(-l1 // 512)
print(f"   level1 tables: {l1}, level2: {l2}, level3: 1, level4(root): 1 -> {(l1+l2+2)*4} KB instead of 512 GB")
print("\n=== Effective access time (memory 100 ns, TLB 10 ns, 1-level table) ===")
m, t = 100, 10
print(f"no TLB: 2 x {m} = {2*m} ns (4-level table: {5*m} ns)")
for h in (0.80, 0.95, 0.99):
    eat = h * (t + m) + (1 - h) * (t + 2 * m)
    eat4 = h * (t + m) + (1 - h) * (t + 5 * m)
    print(f"hit ratio {h:.2f}: EAT = {h}*{t+m} + {1-h:.2f}*{t+2*m} = {eat:.1f} ns  (slowdown {100*(eat/m-1):.0f}% vs {m} ns);  with 4-level table {eat4:.1f} ns")
print("\n=== Internal fragmentation, 100 processes, random sizes 10 KB .. 10 MB (seed 42) ===")
random.seed(42)
sizes = [random.randint(10 * KB, 10 * MB) for _ in range(100)]
tot = sum(sizes)
print(f"total memory requested: {tot/MB:.1f} MB")
rows = []
for name, ps in (("1 KB", KB), ("4 KB", 4 * KB), ("2 MB", 2 * MB)):
    waste = sum((-s) % ps for s in sizes)
    pages = sum(-(-s // ps) for s in sizes)
    pt = pages * 8
    rows.append((name, waste, pages, pt))
    print(f"page {name:>4}: wasted {waste/KB:>10,.0f} KB = {100*waste/tot:6.3f}% of requested; avg {waste/100/KB:8.1f} KB/process (theory ps/2 = {ps/2/KB:.1f} KB); pages {pages:>7,}; page tables {pt/KB:>8,.0f} KB")
import json; json.dump(rows, open("frag_rows.json", "w"))
