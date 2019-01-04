/**
genetic_utils.h
Purpose: Utility funcytions for gen_tsp.cpp

@author Danilo Franco
*/
#include <set>          // collection of distinct elements (used in the generation of a new permutation - nodes unicity)
#include <cmath>        // rand
#include <algorithm>    // random_shuffle, copy, fill

#include "sorting_utils.h"

#define NUMTHREADS 4

/**
Random number generator for the std::random_shuffle method of <algorithm>

@param  i: current element to be swapped

@return Swap element index
*/
int myRand(int i){
    return rand()%i;
}

/**
Compute and return the standard deviation of an array

@param  array: Pointer to the array from which the standard deviation is computed
@param  len: Array length

@return Standard deviation
*/
double stdDev(double *array, int len){
    double avg, var;
    int i;
    
    avg = 0;
    var = 0;
    for(i=0; i<len; ++i)
        avg += array[i];
    avg /= len;
    for(i=0; i<len; ++i)
        var += (array[i]-avg)*(array[i]-avg);
    return sqrt(var/len);
}

/**
Sort an array and apply the same operation to an index array in order to keep track of the sorted row positions

@version 2.0 (quickSort)
@param  generation_rank: Index array
@param  generation_cost: Sorting array
@param  population: Array length 
*/
void sort_vector(int *generation_rank, int *generation_cost, int population){
    int low,high;
    low=0;
    high=population-1;
    
    #pragma omp parallel num_threads(NUMTHREADS)
    #pragma omp single
    mergesort(generation_cost, generation_rank, low, high, NUMTHREADS, population);
    //quickSort(generation_rank, generation_cost, low, high);
}

/**
Compute the permutation cost for the current generation and rank them

@param  generation_rank: Pointer to the index array
@param  generation_cost: pointer to the total permutation cost array
@param  generation: Pointer to the permutation matrix (population*nodes) for the current iteration
@param  cost_matrix: Pointer to memory that contains the symmetric node-travelling cost matrix
@param  numNodes: Number of travelling-nodes in the problem
@param  population: Number of the nodes permutation (possible solution) found at each round
@param  best_num: Number of best elements that will produce the next generation
*/
void rank_generation(int *generation_rank, int *generation_cost, int *generation, int *cost_matrix, int numNodes, int population, int best_num){
    int i,j,source,destination;

    // COST VECTOR COMPUTATION & RANK INITIALISATION
    fill(generation_cost, generation_cost+population, 0);
    // (tests showed that the overhead of handle parallelisation here outweights the benefits even for big matrices 100000x1000)
//#pragma omp parallel for num_threads(NUMTHREADS) private(source,destination,i) schedule(static)
    for(i=0; i<population; ++i){
        // cost of last node linked to the first one
        source = generation[i*numNodes+numNodes-1];
        destination = generation[i*numNodes];
        generation_cost[i] += cost_matrix[source*numNodes+destination];
        // cost of adjacent cells
        for(j=0; j<numNodes-1; ++j){
            source = destination;
            destination = generation[i*numNodes+j+1];
            generation_cost[i] += cost_matrix[source*numNodes+destination];
        }
        
        generation_rank[i]=i;
    }

    sort_vector(generation_rank, generation_cost, population);
    return;
}

/**
Move rows in the generation matrix according to the sorted index array

@param  generation_rank: Pointer to the index array
@param  generation: Pointer to the permutation matrix (population*nodes) for the current iteration
@param  generation_copy: Pointer to the auxiliary permutation matrix (temporarily holds the sorted generation)
@param  numNodes: Number of travelling-nodes in the problem
@param  best_num: Number of best elements (parents) that will produce the next generation
*/
void move_top(int *generation_rank, int *&generation, int *&generation_copy, int numNodes, int best_num){
    int i,*start,*swap;
    for(i=0; i<best_num; ++i){
        start = generation+generation_rank[i]*numNodes;
        copy(start, start+numNodes, generation_copy+i*numNodes); // maybe memcpy is more efficient
    }
    swap = generation;
    generation = generation_copy;
    generation_copy = swap;
}

/**
Generates new permutation from two parents: first half from parent1 and all the remaining from parent2 (in order as well) +
    + mutation: swap between two random nodes

@param  generation: Pointer to the permutation matrix (population*nodes) for the current iteration
@param  parent1: index referring to a row in the generation matrix (read)
@param  parent2: index referring to a row in the generation matrix (read)
@param  son: index referring to a row in the generation matrix (write)
@param  numNodes: Number of travelling-nodes in the problem
@param  mutatProb: probability [0-1] of mutation occurence in the newly generated population element
*/
void crossover_firstHalf_withMutation(int *generation, int parent1, int parent2, int son, int numNodes, int probCentile){
    set<int> nodes;
    int j,k,half,elem,swap1,swap2;

    half = floor(numNodes/2);

    // take first half from parent1
    for(j=0; j<half; ++j){
        elem = generation[parent1*numNodes+j];
        generation[son+j] = elem;
        nodes.insert(elem);
    }
    // add the remaining elements from parent2
    for(k=0; k<numNodes; ++k){
        elem = generation[parent2*numNodes+k];
        if(nodes.find(elem)==nodes.end()){
            generation[son+j] = elem;
            ++j;
        }
    }
    // MUTATION
    if((rand()%100+1)<=probCentile){
        swap1=rand()%numNodes;
        do {
            swap2=rand()%numNodes;
        } while(swap2==swap1);

        elem = generation[son+swap1];
        generation[son+swap1] = generation[son+swap2];
        generation[son+swap2] = elem;
    }
    return;
}

/**
Having the sorted generation matrix, fill it from the last parent index untill the end with the chosen crossover

@param  generation: Pointer to the permutation matrix (population*nodes) for the current iteration
@param  population: Number of the nodes permutation (possible solution) found at each round
@param  best_num: Number of best elements (parents) that will produce the next generation
@param  numNodes: Number of travelling-nodes in the problem
@param  mutatProb: probability [0-1] of mutation occurence in the newly generated population element
*/
void generate(int *generation, int population, int best_num, int numNodes, int probCentile){
    int i,parent1,parent2,son;

    // fill from bestnum until all population is reached
#pragma omp parallel for num_threads(NUMTHREADS) private(parent1,parent2,son,i) schedule(static)
    for(i=0; i<population-best_num; ++i){
        if (i<best_num) // each best must generate at least one son
            parent1 = i;          
        else
            parent1 = rand()%best_num;

        do {    // two different parents
            parent2 = rand()%best_num;
        } while(parent2==i);
        
        son = (best_num+i)*numNodes;

        crossover_firstHalf_withMutation(generation, parent1, parent2, son, numNodes, probCentile);    
    }
}

void transferReceive_bests(int* generation, int* generation_cost, int numNodes, int best_num, int me, int sendTo, int recvFrom){
    int position,buff_size,cost,*permutation;
    char *buffer;
    buff_size = (numNodes+1)*sizeof(int);
    buffer = new char[buff_size];
    permutation = new int[numNodes];

    MPI_Request request;
    MPI_Status status;

    position = 0;

    MPI_Pack(&generation_cost[0], 1, MPI_INT, buffer, buff_size, &position, MPI_COMM_WORLD);
    MPI_Pack(generation, numNodes, MPI_INT, buffer, buff_size, &position, MPI_COMM_WORLD);
    printf("%d send to %d, cost:%d\n",me,sendTo,generation_cost[0]);
    MPI_Isend(buffer, position, MPI_PACKED, sendTo, 0, MPI_COMM_WORLD,&request);

    position = 0;

    MPI_Recv(buffer, buff_size, MPI_PACKED, recvFrom, 0, MPI_COMM_WORLD, &status);
    MPI_Unpack(buffer, buff_size, &position, &cost, 1, MPI_INT, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, buff_size, &position, permutation, numNodes, MPI_INT, MPI_COMM_WORLD);
    printf("%d received from %d; cost: %d\n",me,recvFrom,cost);
    printMatrix(permutation,1,numNodes);

    
    return;
}
