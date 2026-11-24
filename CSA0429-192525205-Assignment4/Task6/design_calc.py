"""Task 6 Stage 4: capacity, throughput and disk-read calculations for the lecture-video platform (all inputs are stated assumptions)."""
GB=1e9; TB=1e12
videos=3000; avg_gb=1.0; bitrate_mbps=4        # catalogue size, average file, typical 720p/1080p mix
students=200_000; peak_share=0.20              # 20 % of students watching in the busiest hour of week 1
print("=== Capacity ===")
raw=videos*avg_gb; print(f"{videos} videos x {avg_gb} GB = {raw/1000:.1f} TB original; with 3 quality renditions (1.0 + 0.5 + 0.25 of original) = {raw*1.75/1000:.2f} TB; 1 backup copy in cloud = {raw*1.75/1000:.2f} TB more")
print("=== Throughput at the peak ===")
conc=students*peak_share*0.75                   # 0.75 hour of viewing per watcher within that hour
print(f"Watchers in the peak hour: {students*peak_share:,.0f}; average concurrent viewers = {conc:,.0f}")
need=conc*bitrate_mbps/8
print(f"Streaming rate = {conc:,.0f} x {bitrate_mbps} Mbit/s = {need:,.0f} MB/s = {need*8/1000:.0f} Gbit/s")
hot_share=0.65
print(f"With Zipf popularity ~{hot_share:.0%} of requests hit the top 40 videos (simulation); those {40*avg_gb:.0f} GB fit in RAM+SSD, so only {1-hot_share:.0%} reach HDD")
print("=== Disk reads to start playing at any minute ===")
print("Start-up = 1 index/metadata read (moov/segment index, ~64 KB at the start of the file) + 1 read of the chunk that contains the chosen minute.")
print("Contiguous (extent) file with an index of chunk offsets: 2 reads, 2 seeks on HDD (~25 ms), 2 flash reads on SSD (<1 ms).")
print("Linked allocation: reaching minute 45 of a 1 GB file (4 KB blocks) = 45/60*262,144 = 196,608 block reads -> impossible; indexed/extent needed.")
print("ext4 extents: inode holds 4 extents; a 1 GB file written once has 8-9 extents (128 MB each) -> 1 extent-tree block + data = 2-3 reads.")
print("=== Number of servers ===")
for name,per in (("HDD server (8 x 7200 rpm, 150 MB/s each, ~60 % usable for streaming)",8*150*0.6),("SSD/NVMe server (2 GB/s usable)",2000),("10 Gbit/s network port",1250*0.8)):
    print(f"{name}: {per:,.0f} MB/s -> servers needed for {need:,.0f} MB/s of misses+hits: {need/per:.1f}")
miss=need*(1-hot_share); print(f"Disk traffic after caching (35 % misses): {miss:,.0f} MB/s -> {miss/(8*150*0.6):.1f} HDD servers; network {need/1000:.1f} GB/s -> {need/1000/1.0:.1f} servers with a 10 Gbit port each: the NETWORK, not the disks, sets the server count, so a few-servers design cannot carry the whole peak -> hybrid with CDN (Stage 5 of the report)")
print("=== Replication ===")
print("Replicas: 2 copies on different servers (RF=2) + 1 cloud backup; storage = 2 x 1.75 x 3 TB = 10.5 TB local. Availability with 99.5 % per server: both copies down = 2.5e-5 (~13 min per year).")
