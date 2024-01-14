#!/bin/bash

# starts: place=pack:excl and place=scatter
#PBS -l select=20:ncpus=1:mem=2gb -l place=pack:excl

# set max execution time
#PBS -l walltime=0:01:00

# define the queue
#PBS -q short_cpuQ

# set error and output folders
#PBS -o outputs
#PBS -e outputs

module load mpich-3.2
# export OMP_PLACES=threads
# export OMP_PROC_BIND=spread
# mpiexec -n 20 --report-bindings --bind-to core --map-by node:pe=2 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -l1000 -p1 -v2

# parallel executions
# mpiexec -n 19 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -r-1 -l1000 -p1 -v0 -b2
mpiexec -n 20 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -r0 -l1000 -p1 -v0 -b1
# mpiexec -n 40 ./project/magic-hexagon/solver.o -n 4 -s3 -M 111 -a 0 -r1 -l1000000 -p1 -o1 -v0 -b3

# sequential executions
# mpiexec -n 1 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -r0 -l1000 -p0 -v0 -b2
# mpiexec -n 1 ./project/magic-hexagon/solver.o -n 3 -M 38 -a 1 -r-1 -l1000 -p0 -v0 -b1

