"""Task 3.2: power failure during a file-system update. Creating a file needs 3 block writes
 (1) data block, (2) inode (size + pointer), (3) bitmap/directory entry. The program crashes after every possible number of completed writes
 and checks consistency after 'reboot', for: no protection, journaling (write-ahead log) and copy-on-write (atomic root switch).
 Also counts the blocks written for a small random update (write amplification)."""
import itertools, json
BLOCKS = ("data", "inode", "dirent_bitmap")
def consistent(state):
    """consistent = (inode points to data) == (bitmap says allocated) == (directory entry exists), and data is complete if referenced"""
    flags = (state["inode"], state["dirent_bitmap"])
    if flags == (True, True): return state["data"]                    # file visible: data must be on disk
    if flags == (False, False): return True                          # file absent: nothing visible (space may be wasted only if data written: harmless)
    return False                                                    # inode without entry (lost inode) or entry without inode (dangling name)
def plain(order, k):
    s = {"data": False, "inode": False, "dirent_bitmap": False}
    for b in order[:k]: s[b] = True
    return s
def journaled(order, k):
    """journal: write the 3 blocks + commit record into the log (4 writes), then checkpoint them in place (3 writes). Recovery replays committed log only."""
    log = {"data": False, "inode": False, "dirent_bitmap": False}; commit = False; s = {"data": False, "inode": False, "dirent_bitmap": False}
    steps = [("log", b) for b in order] + [("commit", None)] + [("home", b) for b in order]
    for kind, b in steps[:k]:
        if kind == "log": log[b] = True
        elif kind == "commit": commit = True
        else: s[b] = True
    if commit:                                                        # recovery: replay the whole transaction (idempotent)
        for b in BLOCKS: s[b] = True
    return s, len(steps)
def cow(order, k):
    """copy-on-write: write new data, new inode, new directory block to FREE locations (3 writes), then ONE atomic write of the root/superblock pointer."""
    s = {"data": False, "inode": False, "dirent_bitmap": False}; root = False
    steps = [("new", b) for b in order] + [("root", None)]
    for kind, b in steps[:k]:
        if kind == "root": root = True
    if root: s = {b: True for b in BLOCKS}
    return s, len(steps)
orders = list(itertools.permutations(BLOCKS))
print("File creation = 3 block writes; the disk may complete the writes in any order (6 orders) and power fails after k of them.\n")
bad_plain = sum(1 for o in orders for k in range(0, 4) if not consistent(plain(o, k)))
tot_plain = len(orders) * 4
print(f"no journal   : {bad_plain} of {tot_plain} crash points leave an INCONSISTENT file system ({100*bad_plain/tot_plain:.0f} %)")
for k in range(0, 4):
    pat = sorted({tuple(b for b in o[:k]) for o in orders if not consistent(plain(o, k))})
    if pat: print(f"   after {k} writes, e.g. only {pat[0]} written -> " + ("directory entry without inode/data (dangling name)" if "dirent_bitmap" in pat[0] and "inode" not in pat[0] else "inode with no name (lost file) or data without inode"))
bad_j = 0; n_j = 0
for o in orders:
    for k in range(0, journaled(o, 0)[1] + 1):
        s, _ = journaled(o, k); n_j += 1
        if not consistent(s): bad_j += 1
print(f"journaling   : {bad_j} of {n_j} crash points inconsistent (every point gives either the old state or the complete new state); log + checkpoint = {journaled(orders[0], 0)[1]} writes instead of 3 (x{journaled(orders[0], 0)[1]/3:.2f})")
bad_c = 0; n_c = 0
for o in orders:
    for k in range(0, cow(o, 0)[1] + 1):
        s, _ = cow(o, k); n_c += 1
        if not consistent(s): bad_c += 1
print(f"copy-on-write: {bad_c} of {n_c} crash points inconsistent; {cow(orders[0], 0)[1]} writes (3 new blocks + 1 atomic pointer write)")
print("\n=== Blocks written for ONE small random update (4 KB) ===")
import math
def cow_tree(depth): return depth + 1 + 1                              # new data block + new copy of every tree node up to the root + root pointer
print("ext4 ordered : 1 data block + 1 metadata block into the journal + the metadata block in place  -> ~3 (journal mode: data also journaled: ~4)")
for n in (10**3, 10**6, 10**9):
    d = math.ceil(math.log(n, 256))                                  # B-tree with 256 pointers per node
    print(f"Btrfs/ZFS-like COW tree with 2^{int(math.log2(n))} blocks (fan-out 256, depth {d}) : 1 data block + {d} tree nodes copied + root = {cow_tree(d)} blocks written; the old blocks stay valid until freed (snapshot)")
