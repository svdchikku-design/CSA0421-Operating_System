"""Task 2.3 / 2.4 calculations: disk reads to reach the last block, maximum inode file size, free-space structures."""
KB, GB, TB = 1024, 1024**3, 1024**4
bs, addr = 4 * KB, 4
fsize = 4 * GB
nblocks = fsize // bs
print(f"File 4 GB, block 4 KB -> {nblocks:,} blocks (2^{nblocks.bit_length()-1}); last block number {nblocks-1:,}")
per = bs // addr
print(f"pointers per indirect block = 4096 / 4 = {per}")
print("\n=== Disk reads to reach the LAST block (the directory entry / FCB is already in memory) ===")
print("contiguous : 1 read (start + k is computed, no pointer is read)")
print(f"linked     : {nblocks:,} reads (each block holds the pointer to the next; the last block is the {nblocks:,}th)")
print(f"FAT (cached in memory): {nblocks:,} memory look-ups + 1 disk read for the data block")
print("FAT (not cached): 1 disk read per FAT sector visited = up to ~%d reads (1,024 entries per 4 KB FAT block, mostly sequential)" % (nblocks // per))
d, s, db = 12, per, per ** 2
print(f"indexed (UNIX inode): direct covers blocks 0..{d-1}; single indirect {d}..{d+s-1}; double indirect {d+s}..{d+s+db-1:,}")
k = nblocks - 1
level = "direct" if k < d else "single" if k < d + s else "double" if k < d + s + db else "triple"
reads = {"direct": 1, "single": 2, "double": 3, "triple": 4}[level]
print(f"  block {k:,} is in the {level} indirect area -> {reads} reads for the pointers+data (inode itself already in memory): double-indirect block, single-indirect block, data block;  4 if the inode must also be read")
print("\n=== Maximum file size with 12 direct + 1 single + 1 double + 1 triple indirect (4 KB blocks, 4 B addresses) ===")
tot = d + s + db + per ** 3
print(f"blocks: {d} + {s} + {db:,} + {per**3:,} = {tot:,}")
print(f"size  : {tot*bs:,} bytes = {tot*bs/TB:.4f} TiB ({tot*bs/GB:,.1f} GiB)")
print(f"        direct only {d*bs//KB} KB; + single {(d+s)*bs/1024**2:.2f} MB; + double {(d+s+db)*bs/GB:.3f} GB; + triple {tot*bs/TB:.3f} TB")
print("(the 32-bit block address also limits it: 2^32 blocks x 4 KB = 16 TB; ext2/3 are limited to 4 TB by the structure above)")
print("\n=== Free-space management for a 4 TB disk, 4 KB blocks ===")
blocks = 4 * TB // bs
print(f"blocks = 4 TB / 4 KB = {blocks:,} = 2^{blocks.bit_length()-1}")
print(f"bit vector = {blocks:,} bits = {blocks//8:,} bytes = {blocks//8/1024**2:.0f} MB  (0.0031 % of the disk)")
print(f"linked free list: one pointer (4 B) kept inside each free block -> no extra space, but finding n free blocks needs n reads; list head only in memory")
print(f"grouping: the first free block holds the addresses of n-1 other free blocks (here {per-1} per block) + pointer to the next group block")
print(f"counting: store (first free block, run length) pairs: a mostly empty disk with r free runs needs 8 B x r  (e.g. 1,000 runs = 8 KB)")
print(f"FAT for 4 TB disk, 4 KB clusters, 4 B entries: {blocks:,} x 4 B = {blocks*4/1024**3:.0f} GB (!)  -> too big for memory; 32 KB clusters: {blocks//8*4/1024**2:.0f} MB")
