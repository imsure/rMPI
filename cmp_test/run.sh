#!/bin/bash

echo "starting $1 .........."
#export PROTOCOL_TYPE=PARALLEL
export PROTOCOL_TYPE=MIRRORED

for i in {1..10}
do
    mpirun -x PROTOCOL_TYPE -np 8 --hostfile hostfile ./$1 8080 30
done
