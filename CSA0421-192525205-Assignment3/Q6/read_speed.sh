#!/bin/bash
# Sequential read speed of a 512 MB file on the only disk in this VM (O_DIRECT bypasses the page cache), then cached read.
f=/tmp/speedfile; dd if=/dev/zero of=$f bs=1M count=512 oflag=direct 2>&1 | tail -1 | sed 's/^/write (direct): /'
dd if=$f of=/dev/null bs=1M iflag=direct 2>&1 | tail -1 | sed 's/^/read, uncached (direct): /'
dd if=$f of=/dev/null bs=1M 2>&1 | tail -1 | sed 's/^/read, 1st buffered: /'
dd if=$f of=/dev/null bs=1M 2>&1 | tail -1 | sed 's/^/read, 2nd buffered (from page cache): /'
echo "Only one virtual disk (vda) is available, so an SSD-vs-HDD comparison is not possible here; typical figures are in the report."
echo; echo '--- swap file: create, format, enable, show, disable'
fallocate -l 64M /tmp/swapfile && chmod 600 /tmp/swapfile && mkswap /tmp/swapfile 2>&1 && swapon /tmp/swapfile && cat /proc/swaps; swapoff /tmp/swapfile; rm -f /tmp/swapfile $f
echo "after swapoff:"; cat /proc/swaps
