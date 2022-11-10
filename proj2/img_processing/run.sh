#! /bin/bash

rNumber=$1
./serial $2 $3
mpirun -np $rNumber -mca btl ^openib -hostfile hosts ./parallel $2 $4