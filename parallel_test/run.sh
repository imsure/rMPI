#!/bin/bash

echo "starting $1 .........."
if [ "$1" == "test_rb" ]
then
    mpirun -np 8 --hostfile hostfile ./$1 8080 30
else
    mpirun -np 8 --hostfile hostfile ./$1
fi