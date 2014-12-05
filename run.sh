#!/bin/bash

echo "starting $1 with $2 protocol"
export PROTOCOL_TYPE=$2
#export PROTOCOL_TYPE=MIRRORED

mpirun -x PROTOCOL_TYPE -np 8 --hostfile hostfile ./$1
