#!/bin/bash
x=`awk '{x += 1; printf("%d ", x);}' bpn.log`
y=`awk '{printf("%s ", $NF);}' bpn.log`
echo "y=[$y];x=[$x];plot(x,y);"
