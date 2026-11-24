# CSA04-192525205-Assignment5

**Course:** CSA0421 – Operating Systems (CSA04) – Assignment 5 (CO5)
**Title:** Deploying the University Result Portal (Linux, virtual machines, containers, cloud)
**Name:** Saai Vardhan D  **Register number:** 192525205

## Environment
| Item | Value |
|---|---|
| Linux | Ubuntu 24.04.5 LTS, kernel 6.18, x86-64, 2 vCPUs, 8 GB RAM (virtual machine, root user) |
| Compiler | gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 (no C code in this assignment; `gcc -std=c11 -Wall -Wextra` rules apply to CO1-CO4) |
| Tools | bash, awk, sort, uniq, ps, free, vmstat, df, iptables, unshare, cgroup v1, python3 + Flask 3.1, curl |
| Not available in my sandbox | Docker daemon, Kubernetes, nginx, hypervisor, internet: the Dockerfile, docker-compose.yml and Kubernetes files are completed but **not executed** there. Run the commands below on a machine with Docker and minikube/kind and add your own screenshots |

## Run
```bash
cd B1 && chmod +x *.sh && ./analyze_log.sh && sudo ./linux_admin.sh
cd ../B2 && sudo ./container_demo.sh        # needs cgroup v1 (on cgroup v2 hosts use systemd-run --scope -p MemoryMax=64M)
cd ../B3 && sudo ./compare_runtimes.sh      # needs python3 -m pip install flask
cd ../B4 && python3 capacity.py
# with Docker + minikube:
cd ../B2 && docker build -t result-portal . && docker run -p 8000:8000 result-portal
cd ../B3 && cat k8s_commands.txt
```

## Folder per task
| Folder | Contents | Result |
|---|---|---|
| B1 | **`analyze_log.sh` (completed TODOs)**, `crontab.txt`, `linux_admin.sh` | output equals expected (200 4 / 302 1 / 403 1 / 404 1; busiest 10.0.0.5 4; 43200 bytes) |
| B2 | **`Dockerfile` (completed TODOs)**, `docker-compose.yml`, `multistage.Dockerfile`, `app.py`, `container_demo.sh` | namespaces + cgroups demo: 64 MB limit kills a 200 MB allocation (exit 137); 50 % CPU quota halves CPU time |
| B3 | **`deployment.yaml` (completed TODOs)**, `service.yaml`, `hpa.yaml`, `k8s_commands.txt`, `compare_runtimes.sh` | bare metal 1,187 req/s; container-style 1,183 (100 % CPU), 591 (50 % CPU) |
| B4 | `notify-deployment.yaml`, `capacity.py` | peak ≈ 521 req/s -> 12 pods with headroom; decision: hybrid (cloud web tier + private database) |

`Report_CO5.docx` is the report; `figures/` holds its diagrams. Values marked *typical* or *assumption* in the report are not measurements.
