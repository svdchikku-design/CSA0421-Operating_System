"""Task 3.2 / alternative: ext4 journaling modes measured on real ext4 (loop-mounted 256 MB images).
Workload: 4 KB random overwrites inside a preallocated 64 MB file, each followed by fsync() (database-style small updates)."""
import os, subprocess, random, time, statistics as S, json
def sh(c): return subprocess.run(c, shell=True, capture_output=True, text=True)
results = {}
for mode in ("journal", "ordered", "writeback"):
    img, mnt = f"/tmp/jm_{mode}.img", f"/tmp/jm_{mode}"
    sh(f"umount {mnt}; rm -f {img}; mkdir -p {mnt}; dd if=/dev/zero of={img} bs=1M count=256 2>/dev/null; mkfs.ext4 -q {img}")
    r = sh(f"mount -o loop,data={mode} {img} {mnt}"); assert r.returncode == 0, r.stderr
    path = f"{mnt}/db.dat"; fd = os.open(path, os.O_RDWR | os.O_CREAT); os.posix_fallocate(fd, 0, 64 << 20); os.fsync(fd)
    buf = os.urandom(4096); random.seed(5); offs = [random.randrange(0, (64 << 20) // 4096) * 4096 for _ in range(3000)]
    runs = []
    for rep in range(3):
        t = time.perf_counter()
        for o in offs: os.pwrite(fd, buf, o); os.fsync(fd)
        runs.append((time.perf_counter() - t) / len(offs) * 1e6)
    t = time.perf_counter()
    for o in offs: os.pwrite(fd, buf, o)
    nosync = (time.perf_counter() - t) / len(offs) * 1e6; os.fsync(fd)
    opts = [l for l in open("/proc/mounts") if mnt in l][0].split()[3]
    results[mode] = (S.median(runs), runs, nosync)
    print(f"data={mode:9s}: 4 KB write + fsync: median {S.median(runs):7.1f} us  (3 runs: {', '.join(f'{x:.0f}' for x in runs)})   write without fsync: {nosync:.1f} us   [mount options: {opts}]")
    os.close(fd); sh(f"umount {mnt}; rm -f {img}")
json.dump(results, open("journal_modes.json", "w"))
base = results["ordered"][0]
print("relative to ordered (default):", ", ".join(f"{m} x{v[0]/base:.2f}" for m, v in results.items()))
