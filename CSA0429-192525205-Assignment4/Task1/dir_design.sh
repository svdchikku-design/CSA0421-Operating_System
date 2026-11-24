#!/bin/bash
# Task 1: flat directory vs tree, and a permission design tested with real users. Needs root (creates two temporary users).
set -u
ROOT=${1:-/tmp/college}; rm -rf "$ROOT"; mkdir -p "$ROOT"; cd "$ROOT"
echo "=== 1. Name clash in the flat directory (two students both save 'assignment1.c') ==="
mkdir flat; echo "student 7001's program" > flat/assignment1.c; echo "student 7002's program" > flat/assignment1.c
echo "flat/assignment1.c now contains: $(cat flat/assignment1.c)   <- the first student's file has been silently overwritten"
echo; echo "=== 2. Tree: /students/<year>/<department>/<register number>/ - the same name is harmless ==="
for r in 7001 7002; do mkdir -p tree/2025/csa/$r; echo "program of $r" > tree/2025/csa/$r/assignment1.c; done
ls -R tree | head -12
echo; echo "=== 3. Search effort for one file among 12,000 students (directory with 12,000 entries vs tree of 24 x 500) ==="
python3 - <<'P'
import os, time, random
random.seed(1); N = 12000
os.makedirs("flat12k", exist_ok=True)
names = [f"{192520000 + i}_notes.txt" for i in range(N)]
t = time.time()
for n in names: open("flat12k/" + n, "w").close()
tc = time.time() - t
for i, n in enumerate(names):
    d = f"tree12k/{i % 24:02d}"
    os.makedirs(d, exist_ok=True); open(f"{d}/{n}", "w").close()
sample = random.sample(range(N), 5000)
def timeit(fn):
    t = time.perf_counter(); [fn(i) for i in sample]; return (time.perf_counter() - t) / len(sample) * 1e6
flat = timeit(lambda i: os.stat("flat12k/" + names[i]))
tree = timeit(lambda i: os.stat(f"tree12k/{i % 24:02d}/{names[i]}"))
print(f"create 12,000 files in one directory: {tc*1000:.0f} ms")
print(f"stat() one file, flat directory of 12,000 entries : {flat:.2f} us  (ext4 hashes directory entries: dir_index)")
print(f"stat() one file, tree (24 directories x 500)       : {tree:.2f} us")
t = time.perf_counter(); l = os.listdir("flat12k"); tl = time.perf_counter() - t
print(f"listing the flat directory: {len(l)} names in {tl*1000:.1f} ms; a person has to scan 12,000 names, in a tree 500")
print("Comparisons for a LINEAR directory (FAT, ext2, old Unix): average n/2 = 6,000 per lookup in the flat directory; 24 + 250 = 274 in the tree; with a hashed directory O(1) in both")
P
echo; echo "=== 4. Permissions: users, groups, private home directories, group project directory ==="
for u in stuA stuB stuC; do id $u >/dev/null 2>&1 || useradd -M -s /bin/bash $u; done
getent group proj1 >/dev/null || groupadd proj1; usermod -aG proj1 stuA; usermod -aG proj1 stuB
mkdir -p home/stuA home/stuB home/stuC projects/proj1
for u in stuA stuB stuC; do chown $u:$u home/$u; chmod 700 home/$u; echo "private notes of $u" > home/$u/notes.txt; chown $u:$u home/$u/notes.txt; chmod 600 home/$u/notes.txt; done
chown root:proj1 projects/proj1; chmod 2770 projects/proj1           # rwx for owner+group, none for others, setgid so new files inherit group proj1
chmod 755 home projects; chmod 751 . 2>/dev/null
ls -ld home/stuA projects/proj1; echo
t() { printf "%-48s" "$1"; shift; out=$(runuser -u "$1" -- "${@:2}" 2>&1) && echo "ALLOWED: $(echo "$out" | head -1)" || echo "DENIED : $(echo "$out" | head -1)"; }
t "stuA reads its own notes"                       stuA cat home/stuA/notes.txt
t "stuB reads stuA's notes"                        stuB cat home/stuA/notes.txt
t "stuC lists stuA's directory"                    stuC ls home/stuA
t "stuA creates a file in the project folder"      stuA sh -c 'echo "design by A" > projects/proj1/design.txt && echo written'
t "stuB (member) reads the project file"           stuB cat projects/proj1/design.txt
t "stuC (not a member) reads the project file"     stuC cat projects/proj1/design.txt
ls -l projects/proj1/design.txt
echo; echo "=== 5. Link for a shared folder: acyclic graph (symbolic link from each member's home to the project) ==="
ln -s "$ROOT/projects/proj1" home/stuA/proj1; ln -s "$ROOT/projects/proj1" home/stuB/proj1
ls -l home/stuA/proj1 home/stuB/proj1
t "stuB reaches the project through its own link"  stuB cat home/stuB/proj1/design.txt
t "stuC cannot follow the same path"               stuC cat "$ROOT/projects/proj1/design.txt"
echo; echo "=== 6. Attributes kept per file (stat) ==="; stat -c 'name=%n  type=%F  size=%s  owner=%U  group=%G  mode=%a (%A)  inode=%i  links=%h  modified=%y' home/stuA/notes.txt
for u in stuA stuB stuC; do userdel $u 2>/dev/null; done; groupdel proj1 2>/dev/null; echo; echo "(temporary users removed)"
