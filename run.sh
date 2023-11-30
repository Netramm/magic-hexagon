#!/bin/bash

#PBS -l select=20:ncpus=1:mem=2gb

# set max execution time
#PBS -l walltime=0:05:00

# define the queue
#PBS -q short_cpuQ

# set error and output folders
#PBS -o outputs
#PBS -e outputs

module load mpich-3.2
mpirun.actual -n 20 ./project/magic-hexagon/serial.o -n 3 -M 38 -a 1 -l1000 -p1
# mpirun.actual -n 1144 ./project/magic-hexagon/serial.o -n 4 -s3 -M 111 -a 0 -l15000 -p1

