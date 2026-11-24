# CSA04-192525205-Assignment4

**Course:** CSA0421 – Operating Systems (CSA04) – Assignment 4 (CO4)
**Title:** Optimising file storage and disk access
**Name:** Saai Vardhan D  **Register number:** 192525205

## Environment
| Item | Value |
|---|---|
| Compiler | gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 |
| Linux | Ubuntu 24.04.5 LTS, kernel 6.18, x86-64, 2 vCPUs (virtual machine) |
| Compile command | `gcc -std=c11 -Wall -Wextra -o prog prog.c` (`-lm` for `disk_sched.c` and `stream_sim.c`; see `build_all.sh`) |
| Tools | mkfs.ext4, tune2fs, dumpe2fs, debugfs, filefrag, stat, df, dd, loop devices, python3 (Btrfs/ZFS/NFS/HDFS were not available: modelled) |

## Build and run
```bash
chmod +x build_all.sh && ./build_all.sh
cd Task2 && ./alloc && ./fat_vs_index
cd ../Task4 && ./sstf_complete && ./disk_sched
cd ../Task6 && ./stream_sim && python3 design_calc.py
```
Scripts that need root: `Task1/dir_design.sh`, `Task3/inspect_fs.sh`, `Task3/journal_modes_bench.py`, `Task5/device_mgmt.sh`. Python: `Task2/cost_calc.py`, `Task3/crash_sim.py`, `Task5/dfs_model.py`, `Task5/single_server_load.py`.

## Folder per task
| Folder | Contents | Result |
|---|---|---|
| Task1 | `dir_design.sh` | flat-directory name clash; tree + permission bits + setgid group folder + symlinks tested |
| Task2 | **`alloc.c` (completed TODO)**, `cost_calc.py`, `fat_vs_index.c` | first-fit output matches expected; 4 GB file = 1,048,576 blocks; max inode file ≈ 4.0039 TiB; 4 TB bit vector = 128 MB |
| Task3 | `inspect_fs.sh`, `journal_modes_bench.py`, `crash_sim.py` | ext4 inspected; journal ×2.70 vs ordered; 10 of 24 crash points inconsistent without journal, 0 with journal or COW |
| Task4 | **`sstf_complete.c` (completed TODO)**, `disk_sched.c` | FCFS 640, SSTF 236, SCAN 236, C-SCAN 183(+199), LOOK 208, C-LOOK 153(+169); SSTF starves request 183 (13,621 ms vs 56.5 ms) |
| Task5 | `dfs_model.py`, `single_server_load.py`, `device_mgmt.sh` | one shared server: response grows linearly; replication model; buffering/caching measured |
| Task6 | `design_calc.py`, `stream_sim.c` | 30,000 concurrent viewers = 120 Gbit/s -> hybrid with CDN; SSD tier p99 68.1 vs 77.3 ms |

`figures/` holds the report figures; `Report_CO4.docx` is the report. Values marked *assumption* in the report are not measurements.
