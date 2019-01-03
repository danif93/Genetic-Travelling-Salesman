#include "mpi.h"

#include <iostream>

using namespace std;

int main (int argc, char **argv) {
    int me,numinstances,i;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &numinstances);
    printf("\nme: %d;\tnum instances: %d\n",me,numinstances);
    return 0;
}