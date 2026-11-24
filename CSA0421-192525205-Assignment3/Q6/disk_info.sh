#!/bin/bash
echo '$ lsblk'; lsblk
echo; echo '$ cat /proc/partitions'; cat /proc/partitions
echo; echo '$ cat /proc/swaps'; cat /proc/swaps; echo "(no swap configured on this VM: SwapTotal = $(grep SwapTotal /proc/meminfo | tr -s ' '))"
echo; echo '$ df -hT /'; df -hT / | head -3
echo; echo '$ fdisk -l'; (fdisk -l 2>&1 || echo "fdisk is not installed in this VM (util-linux fdisk); lsblk and /proc/partitions show the same partition table")
echo; echo '$ cat /sys/block/*/queue/rotational  (0 = SSD, 1 = hard disk)'; for d in /sys/block/*/queue/rotational; do echo "$d: $(cat $d)"; done 2>/dev/null | grep -v loop | head
