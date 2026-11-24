#!/bin/bash
# B1 - Complete the Code: summarise a web-server access log with awk, sort and uniq.
set -euo pipefail
LOG=$(mktemp)
trap 'rm -f "$LOG"' EXIT
cat > "$LOG" <<'EOF'
10.0.0.5 - - [24/Nov/2026:10:00:01 GET /result/192525205 200 12000
10.0.0.5 - - [24/Nov/2026:10:00:02 GET /result/192525206 200 11000
10.0.0.7 - - [24/Nov/2026:10:00:02 GET /login 302 500
10.0.0.5 - - [24/Nov/2026:10:00:03 GET /admin 403 1200
10.0.0.9 - - [24/Nov/2026:10:00:04 GET /result/999 404 1500
10.0.0.9 - - [24/Nov/2026:10:00:05 GET /result/192525207 200 9000
10.0.0.5 - - [24/Nov/2026:10:00:06 GET /result/192525208 200 8000
EOF
echo "Requests per status code:"
# TODO 1 (completed): count every status code (field 7) and print it as "  200 4", sorted by code
awk '{count[$7]++} END {for (code in count) printf "  %s %d\n", code, count[code]}' "$LOG" | sort
echo "Busiest client:"
# TODO 2 (completed): the client (field 1) with most requests, printed as "  10.0.0.5 4"
awk '{print $1}' "$LOG" | sort | uniq -c | sort -rn | head -1 | awk '{printf "  %s %s\n", $2, $1}'
echo -n "Total bytes sent: "
awk '{s += $8} END {print s}' "$LOG"
