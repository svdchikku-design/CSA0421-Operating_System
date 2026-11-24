#!/bin/bash
# B3: start-up time, memory and requests/second of the portal run (a) directly on the host and (b) in a container-style sandbox
# (own PID/mount/UTS namespaces + cgroup limits). Docker and a hypervisor are not available in this sandbox.
set -u; cd "$(dirname "$0")"; CG=/sys/fs/cgroup
measure() {   # $1 label  $2 port ; $START starts the server
  local t0=$(date +%s%N)
  eval "$START" >/dev/null 2>&1 &
  for i in $(seq 1 300); do curl -s -o /dev/null http://127.0.0.1:$2/health && break; sleep 0.02; done
  local t1=$(date +%s%N)
  local real=$(pgrep -f "^python3 app.py $2" | head -1)
  printf '%-36s start-up %4d ms | RSS %6s kB | ' "$1" $(( (t1-t0)/1000000 )) "$(awk '/VmRSS/{print $2}' /proc/$real/status)"
  timeout 30 python3 load_gen.py $2 8 6
  pkill -9 -f "^python3 app.py $2"; while curl -s -o /dev/null http://127.0.0.1:$2/health; do sleep 0.1; done; sleep 0.5
}
echo "host: $(nproc) CPUs; load: 8 keep-alive clients for 6 s; Flask development server (threaded); gunicorn is not installed here"
START="python3 app.py 8101"; measure "bare metal (process on host)" 8101
for q in 100000 50000; do
 mkdir -p $CG/cpu/c5 $CG/memory/c5; echo 100000 > $CG/cpu/c5/cpu.cfs_period_us; echo $q > $CG/cpu/c5/cpu.cfs_quota_us; echo $((256*1024*1024)) > $CG/memory/c5/memory.limit_in_bytes
 START="unshare --pid --fork --mount-proc --uts --ipc sh -c 'echo \$\$ > $CG/cpu/c5/cgroup.procs; echo \$\$ > $CG/memory/c5/cgroup.procs; exec python3 app.py 8102'"
 measure "container-style, limit $((q/1000)) % of 1 CPU" 8102; rmdir $CG/cpu/c5 $CG/memory/c5 2>/dev/null
done
