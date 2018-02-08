# Simultaneous Iteration Method

## Authors
* Daumen Anton
* Derumigny Nicolas


## Description
This is the repository of the MPNA project of the M2 CHPS (University of Versailles). It consists of the computation of eigenvalues of sparse matrices accelerated by GPU. It uses the Arnoldi method to project the initial matrix in the Krylov subspace, then find the eigenvalues utilse the simultaneos iteration method.

It is parallized by intitializing the vector for the Krylov subspace to random values and picking the result that realise the minimum error rate.


## Required libraries
* A working C compiler
* OpenCL 1.2
* MPI
* Doxygen and Graphviz for documentation generation


## Compiling

```
mkdir build
cd build
cmake ..
make
```


## Executing

```
mpirun -n num_process SimultIte {-i | --infile} infile {-n | --num} number_of_eigenvalues [-h]
```


## Building documentation

```
make doc
```
Browse the documentation by opening the file *build/docs/html/index.html*.


## Credit

* [Matrix Market Library](http://math.nist.gov/MatrixMarket/mmio-c.html)
