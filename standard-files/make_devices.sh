#!/bin/sh

for i in `seq 1 10` ; do
mknod dev$i c 1 $i
mknod bdev$i b 2 $i
done
