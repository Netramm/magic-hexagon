#!/bin/bash

#PBS -l select=20:ncpus=3:mem=2gb

# set max execution time
#PBS -l walltime=0:05:00

# define the queue
#PBS -q short_cpuQ

# set error and output folders
#PBS -o outputs
#PBS -e outputs

module load mpich-3.2
# module load openmpi-4.0.4
# export OMP_PLACES=threads
# mpiexec -n 20 --report-bindings --bind-to core --map-by node:pe=1 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -l1000 -p1
mpiexec -n 20 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -l1000 -p1
# mpiexec -n 2 --report-bindings --bind-to core --map-by node:pe=2 ./project/magic-hexagon/test.o
# mpiexec -n 1 --map-by node:PE=2 --bind-to core ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -l1000 -p1
# mpiexec -n 858 ./project/magic-hexagon/solver.o -n 4 -s3 -M 111 -a 1 -l15000 -p1 -o1

