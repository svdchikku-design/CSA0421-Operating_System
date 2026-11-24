#!/bin/bash
# Q1 experiments. Run inside Q1 after: ../build_all.sh  (or the gcc commands in README)
cd "$(dirname "$0")"
cp libenc_v1.so libenc.so
echo "=== 1. Executable sizes (bytes) ==="; ls -l bank_static bank_dynamic bank_dlopen libenc.so | awk '{printf "%-14s %8d\n",$9,$5}'
echo; echo "=== 2. ldd ==="; for p in bank_static bank_dynamic bank_dlopen; do echo "--- $p"; ldd ./$p 2>&1; done
echo; echo "=== 3. Output of each version ==="; ./bank_static; ./bank_dynamic; ./bank_dlopen
echo; echo "=== 4. Load addresses from /proc/self/maps (dynamic version) ==="; ./bank_dynamic maps | grep -v '^cipher\|^libenc\|^&main'
echo; echo "=== 4b. Run twice: ASLR gives different addresses each time ==="; for i in 1 2; do ./bank_dynamic | grep '&main'; done
echo; echo "=== 5. Start-up time: 2000 runs each ==="
for p in bank_static bank_dynamic "bank_dlopen ./libenc.so"; do
  s=$(date +%s.%N); for i in $(seq 2000); do ./$p >/dev/null; done; e=$(date +%s.%N)
  echo "$p: $(echo "($e-$s)*1000/2000" | bc -l | cut -c1-6) ms per run"
done
echo; echo "=== 6. Memory of one running copy (kB, from /proc/<pid>/smaps_rollup) ==="
for p in bank_static bank_dynamic; do ./$p hold >/dev/null & sleep 0.5; pid=$!
  echo "$p: $(grep -E '^(Rss|Pss|Private_Dirty|Private_Clean|Shared_Clean)' /proc/$pid/smaps_rollup | tr -s ' ' | tr '\n' ' ')"; wait; done
echo; echo "=== 7. Two copies of bank_dynamic: which pages are shared? (smaps, kB) ==="
./bank_dynamic hold >/dev/null & p1=$!; ./bank_dynamic hold >/dev/null & p2=$!; sleep 0.7
for f in libenc.so libc.so.6 bank_dynamic; do
  echo "--- mapping of $f in copy 1 (all segments summed)"; 
  awk -v f="$f" '$0 ~ f && /^[0-9a-f]+-/ {on=1; next} /^[0-9a-f]+-/ {on=0} on && /^(Rss|Pss|Shared_Clean|Shared_Dirty|Private_Clean|Private_Dirty):/ {a[$1]+=$2} END {for (k in a) printf "%s %d kB   ", k, a[k]; print ""}' /proc/$p1/smaps
done
echo "Per-process PSS of the whole copies:"; for p in $p1 $p2; do grep -E '^(Rss|Pss)' /proc/$p/smaps_rollup | tr -s ' ' | tr '\n' ' '; echo; done
wait
echo; echo "=== 8. Replace the library WITHOUT recompiling ==="
cp libenc_v2.so libenc.so
echo "bank_dynamic (shared lib replaced):"; ./bank_dynamic; echo "bank_static (still contains old code):"; ./bank_static; echo "bank_dlopen:"; ./bank_dlopen
echo; echo "=== 9. Library missing ==="
mv libenc.so libenc.so.hidden
echo "--- bank_dynamic:"; ./bank_dynamic; echo "exit status $?"
echo "--- bank_static:"; ./bank_static | head -1; echo "exit status $?"
echo "--- bank_dlopen:"; ./bank_dlopen; echo "exit status $?"
mv libenc.so.hidden libenc.so; cp libenc_v1.so libenc.so
