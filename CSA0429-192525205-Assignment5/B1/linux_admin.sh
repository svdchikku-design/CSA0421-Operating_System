#!/bin/bash
# B1: administer the Linux server - processes, memory, disk, network, users and permissions, firewall, service unit
set -u
echo "== OS and kernel =="; . /etc/os-release; echo "$PRETTY_NAME, kernel $(uname -r), $(nproc) CPUs"
echo; echo "== Processes: top 5 by memory (ps) =="; ps -eo pid,user,%cpu,%mem,rss,comm --sort=-rss | head -6
echo; echo "== Process states summary =="; ps -eo stat= | cut -c1 | sort | uniq -c
echo; echo "== Memory (free -m) =="; free -m
echo; echo "== CPU and memory activity (vmstat 1 3) =="; vmstat 1 3
echo; echo "== Load average and uptime =="; cat /proc/loadavg
echo; echo "== Disk space and inodes =="; df -hT / | tail -2; df -i / | tail -2
echo; echo "== Network interfaces =="; ip -brief addr 2>/dev/null || cat /proc/net/dev | head -4
echo; echo "== Users, groups and permissions =="
useradd -m -s /bin/bash portaladm 2>/dev/null; useradd -r -s /usr/sbin/nologin portalsvc 2>/dev/null
id portaladm; id portalsvc
mkdir -p /tmp/portal_data && chown portalsvc:portalsvc /tmp/portal_data && chmod 750 /tmp/portal_data
ls -ld /tmp/portal_data
echo "result data" > /tmp/portal_data/results.csv; chown portalsvc:portalsvc /tmp/portal_data/results.csv; chmod 640 /tmp/portal_data/results.csv
echo -n "as portaladm (other user), read results.csv: "; su -s /bin/bash portaladm -c 'cat /tmp/portal_data/results.csv' 2>&1 | head -1
echo -n "as portalsvc (owner), read results.csv: "; su -s /bin/bash portalsvc -c 'cat /tmp/portal_data/results.csv' 2>&1 | head -1
echo; echo "== Firewall (iptables) inside a private network namespace: allow SSH 22, HTTP 80, HTTPS 443, drop the rest =="
unshare --net sh -c '
iptables -P INPUT DROP; iptables -A INPUT -i lo -j ACCEPT
iptables -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT 2>/dev/null || iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
for p in 22 80 443; do iptables -A INPUT -p tcp --dport $p -j ACCEPT; done
iptables -L INPUT -n --line-numbers'
echo; echo "== Service unit (systemd) for the portal =="
cat > /tmp/result-portal.service <<'EOF'
[Unit]
Description=University Result Portal
After=network.target
[Service]
User=portalsvc
WorkingDirectory=/opt/portal
ExecStart=/usr/bin/gunicorn --workers 4 --bind 127.0.0.1:8000 app:app
Restart=on-failure
RestartSec=2
LimitNOFILE=65536
[Install]
WantedBy=multi-user.target
EOF
cat /tmp/result-portal.service
echo; echo "== Cleanup =="; userdel -r portaladm 2>/dev/null; userdel portalsvc 2>/dev/null; rm -rf /tmp/portal_data; echo done
