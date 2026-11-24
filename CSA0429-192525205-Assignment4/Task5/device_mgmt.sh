#!/bin/bash
# Task 5.3: block-device interface, buffering and caching, spooling (real commands on this machine)
set -u
D=$(mktemp -d); cd "$D"
echo "== Devices seen by the kernel (block vs character) =="
ls -l /dev/vda /dev/null /dev/tty 2>&1 | awk '{print $1, $5","$6, $NF}'
echo; echo "== Block device queue parameters =="
for f in logical_block_size physical_block_size rotational read_ahead_kb nr_requests; do printf '%s = ' $f; cat /sys/block/vda/queue/$f; done
echo; echo "== Driver / interrupts (first lines of /proc/interrupts, /proc/devices) =="
head -4 /proc/interrupts | cut -c1-90; grep -E 'Block|vd|sd' /proc/devices | head
echo; echo "== Buffering: write 64 MB with 4 KB writes, buffered vs O_DIRECT vs fsync each block (256 blocks) =="
dd if=/dev/zero of=f1 bs=4k count=16384 2>&1 | tail -1
dd if=/dev/zero of=f2 bs=4k count=16384 oflag=direct 2>&1 | tail -1
dd if=/dev/zero of=f3 bs=4k count=256 oflag=dsync 2>&1 | tail -1
echo; echo "== Caching: read the same 64 MB file, cold (cache dropped) then warm (page cache) =="
sync; echo 3 > /proc/sys/vm/drop_caches 2>/dev/null && echo "(page cache dropped)"
dd if=f1 of=/dev/null bs=1M 2>&1 | tail -1
dd if=f1 of=/dev/null bs=1M 2>&1 | tail -1
echo; echo "== Page cache size now (free -m) =="; free -m | head -2
echo; echo "== Spooling: jobs go to a queue directory and one 'daemon' drains it in order, so the slow device is used by one job at a time =="
mkdir spool; for i in 1 2 3; do echo "job$i data" > spool/$i.job; done
for j in $(ls spool | sort -n); do echo "printing $j: $(cat spool/$j)"; rm spool/$j; done
cd /; rm -rf "$D"
