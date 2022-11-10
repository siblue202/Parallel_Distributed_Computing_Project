#! /bin/bash

# rNumber=$(shuf -i 1-20 -n1)
# echo "random nubmer : ${rNumber}"
rNumber=$1
mpirun -np $rNumber -mca btl ^openib -hostfile hosts ./mpi_scan
mpirun -np $rNumber -mca btl ^openib -hostfile hosts ./mpi_block
mpirun -np $rNumber -mca btl ^openib -hostfile hosts ./mpi_nonblock