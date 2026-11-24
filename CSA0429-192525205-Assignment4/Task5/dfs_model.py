"""Task 5: capacity, storage and availability model: one NFS server versus a replicated distributed file system (6 nodes, 3 replicas)."""
print("=== Assumptions: node = 1 server with 10 Gbit/s network (1,250 MB/s) and disks delivering 800 MB/s; 100 TB of useful data; node availability 99.5 % ===")
node_mbs, nodes, rep = 800, 6, 3
print(f"NFS (1 server): read capacity {node_mbs} MB/s, write capacity {node_mbs} MB/s (all clients share it)")
print(f"DFS ({nodes} nodes, {rep} replicas): read capacity ~ {nodes*node_mbs} MB/s (every replica can serve a read: x{nodes}); write capacity ~ {nodes*node_mbs//rep} MB/s (each byte is written {rep} times: x{nodes}/{rep})")
print("\n=== Response time as users grow (M/M/1, 1 MB reads, 1 request per user per 10 s) ===")
mu1 = node_mbs / 1.0                               # requests/s one server can serve for 1 MB requests
print(f"{'users':>7} {'NFS: load':>10} {'NFS: response (ms)':>20} {'DFS: load':>10} {'DFS: response (ms)':>20}")
for users in (100, 1000, 4000, 6000, 7000, 7900, 20000, 40000):
    lam = users / 10
    row = f"{users:>7}"
    for cap in (mu1, mu1 * nodes):
        rho = lam / cap
        row += f" {rho:>9.0%} " + (f"{1000/(cap-lam):>19.1f} " if rho < 1 else f"{'overloaded':>19} ")
    print(row)
A = 0.995
print("\n=== Availability ===")
d1 = (1 - A) * 8760
print(f"NFS, one server: availability {A:.3%} -> down {d1:.1f} hours per year ({(1-A)*100:.1f} % of the time); all users affected")
for k in (2, 3):
    u = (1 - A) ** k
    print(f"{k} independent replicas: data unavailable only if all {k} fail together: unavailability {u:.2e} -> {u*8760*3600:.1f} seconds per year (in practice: correlated failures such as one rack or power feed make it larger)")
print("\n=== Storage used for 100 TB of data ===")
print(f"NFS, 1 copy: 100 TB (+ RAID overhead e.g. RAID 6 on 8 disks: 133 TB raw);   DFS with {rep} replicas: {100*rep} TB raw ({rep}x);   erasure coding 6+3: {100*9/6:.0f} TB (1.5x) with the same two-failure tolerance")
