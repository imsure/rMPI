#!/bin/bash

echo "starting $1 .........."
#export PROTOCOL_TYPE=PARALLEL
export PROTOCOL_TYPE=MIRRORED
if [ "$1" == "test_rb" ]
then
    mpirun -x PROTOCOL_TYPE -np 8 --hostfile hostfile ./$1 8080 30
else
    mpirun -x PROTOCOL_TYPE -np 8 --hostfile hostfile ./$1
fi