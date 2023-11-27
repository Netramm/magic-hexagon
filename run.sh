#!/bin/bash

#PBS -l select=1:ncpus=1:mem=2gb

# set max execution time
#PBS -l walltime=0:10:00

# define the queue
#PBS -q short_cpuQ

# set error and output folders
#PBS -o outputs
#PBS -e outputs

./project/magic-hexagon/serial.o 3 38 1 1000
# ./project/magic-hexagon/serial.o 4 3 111 0 15000
