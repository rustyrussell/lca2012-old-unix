#!/bin/sh
#
# Params:  <path to analyse binary> <path to read input files>
#
AN=$1
INPUT=$2

for i in `ls $INPUT/* | sed -e "s/......$//;" | sort | uniq` ;
do
    echo "--- $i"
    $AN -q ${i}* 
done
