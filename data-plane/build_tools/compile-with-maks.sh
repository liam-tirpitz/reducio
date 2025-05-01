#!/bin/bash
DATA="../"

for i in {0..31}
do
	print "########## Compile $i / 31 ##########"
	mask=$((0xFFFFFFFF >> i))
	mask=$((mask << i))
	mask=$(printf "0x%02X" $mask)
	cp $DATA/Step12/combinedcluster.p4 $DATA/Step12/combinedcluster-$mask.p4
	./p4-compile-9.7.0.sh  $DATA/Step12/combinedcluster-$mask.p4 MASK=32w$mask
	rm $DATA/Step12/combinedcluster-$mask.p4
done

