# Parallel Magic Hexagon

This project was developed for the course High Performance Computing for Data Science. Its aim is to implements a parallel solver for the Magic Hexagon using MPI and OpenMP.

In order to build the solver execute the following code command:
```
mpicc -g -Wall -fopenmp -o solver.o solver.c helpers.c
```