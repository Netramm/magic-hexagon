#!/bin/bash

#PBS -l select=1:ncpus=1:mem=2gb

# set max execution time
#PBS -l walltime=0:05:00

# define the queue
#PBS -q short_cpuQ

# set error and output folders
#PBS -o outputs
#PBS -e outputs

./project/magic-hexagon/serial.o -n 3 -M 38 -a 1 -l1000
# ./project/magic-hexagon/serial.o -n 4 -s3 -M 111 -a 0 -l15000
