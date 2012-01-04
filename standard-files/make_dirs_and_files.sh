#!/bin/sh

for i in `seq 1 100` ; do
echo $i > $i
done

for i in `seq 1 10` ; do
mkdir -p dir$i
done

# Some variety:
chmod 000 dir2
# 
touch  -t 196901010100 dir1
