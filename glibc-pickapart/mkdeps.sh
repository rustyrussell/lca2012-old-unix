#!/bin/sh

mkdir -p deps

TEMP=`mktemp`

for i in *.o 
do
    ODIR=deps/${i%%.o}
    mkdir -p $ODIR 
    nm $i > $TEMP
    egrep " U " $TEMP | sed -e "s/^.* //;" > $ODIR/in
    egrep -v " U " $TEMP | sed -e "s/^.* //;" > $ODIR/out
done
