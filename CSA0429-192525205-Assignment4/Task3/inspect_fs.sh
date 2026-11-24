#!/bin/bash
# Task 3.4: inodes, block size and free space of a real ext4 file system (a 128 MB image mounted through a loop device) and of /.
set -u
IMG=/tmp/ext4demo.img; MNT=/tmp/ext4demo
umount $MNT 2>/dev/null; rm -f $IMG; mkdir -p $MNT
dd if=/dev/zero of=$IMG bs=1M count=128 2>/dev/null
mkfs.ext4 -q -L demo $IMG && mount -o loop $IMG $MNT
echo "hello file system" > $MNT/hello.txt; head -c 100000 /dev/urandom > $MNT/data.bin; mkdir $MNT/dir; ln $MNT/hello.txt $MNT/hardlink.txt; ln -s hello.txt $MNT/symlink.txt
echo '$ ls -li /tmp/ext4demo        (-i = inode number; hardlink.txt shares the inode of hello.txt)'; ls -li $MNT
echo; echo '$ stat hello.txt data.bin'; stat $MNT/hello.txt $MNT/data.bin
echo; echo '$ df -h  and  df -i  (blocks and inodes)'; df -h $MNT /; df -i $MNT /
echo; echo '$ tune2fs -l (selected fields)'; tune2fs -l $IMG | grep -E "Filesystem (features|volume name|magic|state)|Inode count|Free inodes|Block count|Free blocks|Block size|Inodes per group|Inode size|Journal|Default mount|Filesystem OS"
echo; echo '$ dumpe2fs -h (journal fields)'; dumpe2fs -h $IMG 2>/dev/null | grep -iE "journal|features"
sync; echo; echo '$ filefrag -v data.bin   (extents = how ext4 allocates)'; filefrag -v $MNT/data.bin | head -8
echo; echo '$ debugfs -R "stat <inode>"   (the inode of data.bin: extent tree instead of 12+3 pointers)'; debugfs -R "stat /data.bin" $IMG 2>&1 | head -22
echo; echo '$ mount | grep demo'; mount | grep ext4demo
echo; echo '$ root file system'; df -hT / | tail -1; stat -f / | head -4
umount $MNT; rm -f $IMG
