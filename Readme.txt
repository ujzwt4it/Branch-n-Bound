Group Information :

   Bhuvnesh Jain   2014A7PS028P
   Chirag Agarwal  2014A7PS033P

   Problem Statement : Branch And Bound
   Problems Selected : Assignment Problem and TSP
   

The project contains implementations of OpenMP and MPI solutions to well-known Branch and Bound problems-

1.Assignment Problem
2.Travelling Salesman Problem


To run OpenMP version for Assignment Problem
-----------------------------------------------------------------------------------------------------------------------
In the Assignment Problem Sub-directory;
g++ -fopenmp assignment_OpenMP.cpp 
./a.out < inp.txt num_of_threads

where inp.txt is the input file and num_of_threads is the number of threads on which we wish to run the above program
-----------------------------------------------------------------------------------------------------------------------


To run MPI version for Assignment Problem
-----------------------------------------------------------------------------------------------------------------------
In the Assignment Problem Sub-directory;
mpic++ assignment_MPI.cpp
mpirun -np x ./a.out inp.txt

where x is the number of processors and inp.txt is the input file
-----------------------------------------------------------------------------------------------------------------------

To run the sequential code for Assignment Problem just set the number of threads as 1 in openmp version.

Similar procedure can be done in order to run the Travelling Salesman Problem solution, except that file names differ.

To find the results of selected test-cases (generated using the test_generator.cpp) present in folder corresponding to 
both problems, refer to the file

--Analysis.pdf

**********************************************END*********************************************************************** 