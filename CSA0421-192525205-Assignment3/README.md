# CSA04-192525205-Assignment3

**Course:** CSA0421 – Operating Systems (CSA04) – Assignment 3 (CO3)
**Title:** Evaluating memory-management mechanisms: linking, paging, segmentation, virtual memory, page replacement and disk management
**Name:** Saai Vardhan D  **Register number:** 192525205

## Environment
| Item | Value |
|---|---|
| Compiler | gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 |
| Linux | Ubuntu 24.04.5 LTS, kernel 6.18, x86-64, 2 vCPUs (virtual machine), page size 4 KB |
| Compile command | `gcc -std=c11 -Wall -Wextra -pthread -o prog prog.c` (Q1 adds `-fPIC -shared`, `-static`, `-ldl`; see `build_all.sh`) |
| Tools | ldd, /proc/<pid>/maps and smaps, getrusage, lsblk, dd, python3 (`/usr/bin/time -v` and `fdisk` are not installed in my VM) |

## Build and run
```bash
chmod +x build_all.sh && ./build_all.sh
cd Q1 && ./run_q1.sh            # all Q1 experiments
cd ../Q5 && ./page_replace      # completed LRU program
```
Run each program from inside its own Q folder. Python scripts: `python3 page_calcs.py` etc. Q7: `./rt_lock`, `./rt_lock lock`, `./rt_lock lock fifo`, `python3 edge_sim.py`.

## Folder per question
| Folder | Contents | Result |
|---|---|---|
| Q1 | `bank.c`, `bank_dlopen.c`, `encrypt_v1.c`, `encrypt_v2.c`, `run_q1.sh` | static 771 KB vs dynamic 16 KB; 2 copies: Pss 631 kB vs 1,488 kB; library replaced without recompiling; missing library -> exit 127 (dynamic) vs controlled error (dlopen) |
| Q2 | **`page_translate.c` (completed TODO)**, `boundary_test.c`, `ipt_vs_multilevel.c`, `page_calcs.py`, `linux_pages.sh` | output equals the expected output; page-table 4 MB (32-bit) / 512 GB (48-bit flat); EAT 130/115/111 ns; waste 0.010 / 0.042 / 21.2 % for 1 KB / 4 KB / 2 MB pages |
| Q3 | `segment.c`, `segfault_demo.c`, `maps_demo.c`, `seg_frag.py` | 8 addresses verified; SIGSEGV explained; external fragmentation 53.5 % |
| Q4 | `eat_calc.py`, `touch_pages.c`, `limited_ram.c`, `cow_demo.c`, `refstring_faults.py` | EAT 8,099.9 / 180 / 108 ns; max fault rate for 10 % slow-down 1 in 799,990; COW fork 2.6 ms for 200 MB |
| Q5 | **`page_replace.c` (completed TODO)**, `page_sim.c` | FIFO 15/10, LRU 12/8, Optimal 9/8; Belady 9 -> 10; locality: LRU up to 45 % fewer faults than FIFO |
| Q6 | `disk_info.sh`, `read_speed.sh`, `raid_calcs.py`, `parity_bench.c` | RAID capacities 24/12/20/16/12 TB; RAID 5 rebuild error risk 79.8 % with 4 TB consumer disks; recommend RAID 6 |
| Q7 | `rt_lock.c`, `edge_sim.py` | locked loop: 12 page faults vs 47,970 unlocked; RT task never faults in the simulation |

## Commits
Commit after finishing each question; `commit_each_task.sh` shows the intended one-commit-per-question order.

## Honesty of the evidence
Measurements come from a 2-vCPU VM without swap; Q4 and Q7 emulate memory pressure inside the program (explained in the report). Re-run on your own machine for your screenshots; values marked "assumption" in the report are typical figures, not measurements.
