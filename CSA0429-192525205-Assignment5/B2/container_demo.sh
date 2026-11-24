#!/bin/bash
# B2/B3: what a container really is - namespaces (isolation) + cgroups (limits) + a filesystem - done by hand (no Docker daemon in this sandbox)
set -u
cd "$(dirname "$0")"
CG=/sys/fs/cgroup
echo "== 1. Namespaces: the process sees its own PID 1 and hostname =="
unshare --pid --fork --mount-proc --uts --ipc sh -c 'hostname result-portal; echo "hostname=$(hostname) pid=$$"; ps -e | head -3'
echo "host hostname unchanged: $(hostname)"
echo; echo "== 2. Network namespace: separate, empty network stack =="
unshare --net sh -c 'cat /proc/net/dev | tail -n +3 | cut -d: -f1'
echo; echo "== 3. cgroup memory limit: a 64 MB limit kills a process that allocates 200 MB (what --memory does) =="
mkdir -p $CG/memory/demo; echo $((64*1024*1024)) > $CG/memory/demo/memory.limit_in_bytes
sh -c "echo \$\$ > $CG/memory/demo/cgroup.procs; exec python3 -c 'b=bytearray(200*1024*1024); print(\"allocated\")'"; echo "exit status: $? (137 = killed by SIGKILL, the out-of-memory killer)"
echo "memory.max_usage_in_bytes = $(cat $CG/memory/demo/memory.max_usage_in_bytes) ; failcnt = $(cat $CG/memory/demo/memory.failcnt)"
rmdir $CG/memory/demo 2>/dev/null
echo; echo "== 4. cgroup CPU limit: 0.5 CPU (cpu.cfs_quota_us 50000 per 100000) on a CPU-bound loop for 3 s =="
mkdir -p $CG/cpu/demo; echo 100000 > $CG/cpu/demo/cpu.cfs_period_us
for q in -1 50000; do echo $q > $CG/cpu/demo/cpu.cfs_quota_us
  sh -c "echo \$\$ > $CG/cpu/demo/cgroup.procs; exec python3 -c '
import time; t=time.process_time(); w=time.time(); 
while time.time()-w<3: pass
print(\"quota=$q  cpu seconds used in 3 s wall: %.2f\" % (time.process_time()-t))'"
done
rmdir $CG/cpu/demo 2>/dev/null
