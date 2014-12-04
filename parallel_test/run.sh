#!/bin/bash

echo "starting $1 .........."
if [ "$1" == "test_rb" ]
then
    mpirun -np 8 --hostfile hostfile ./$1 2800 30
else
    mpirun -np 8 --hostfile hostfile ./$1
fi