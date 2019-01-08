/**
genetic_utils.h
Purpose: Utility funcytions for gen_tsp.cpp

@author Danilo Franco
*/
#include <set>          // collection of distinct elements (used in the generation of a new permutation - nodes unicity)
#include <cmath>        // rand
#include <algorithm>    // random_shuffle, copy, fill

#include "sorting_utils.h"

#define PRINTSCOST

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
void sort_vector(int *generation_rank, int *generation_cost, int population, int numThreads){
    int low,high;
    low=0;
    high=population-1;
    
    #pragma omp parallel num_threads(numThreads)
    #pragma omp single
    mergesort(generation_cost, generation_rank, low, high, numThreads);
    //quickSort(generation_rank, generation_cost, low, high);
}

/**
Move rows in the generation matrix according to the sorted index array

@param  generation_rank: Pointer to the index array
@param  generation: Pointer to the permutation matrix (population*nodes) for the current iteration
@param  generation_copy: Pointer to the auxiliary permutation matrix (temporarily holds the sorted generation)
@param  numNodes: Number of travelling-nodes in the problem
@param  bestNum: Number of best elements (parents) that will produce the next generation
*/
void move_top(int *generation_rank, int *&generation, int *&generation_copy, int numNodes, int bestNum){
    int i,*start,*swap;
    for(i=0; i<bestNum; ++i){
        start = generation+generation_rank[i]*numNodes;
        copy(start, start+numNodes, generation_copy+i*numNodes);
    }
    swap = generation;
    generation = generation_copy;
    generation_copy = swap;
}

/**
Compute the permutation cost for the current generation and rank them

@param  generation_rank: Pointer to the index array
@param  generation_cost: pointer to the total permutation cost array
@param  generation: Pointer to the permutation matrix (population*nodes) for the current iteration
@param  cost_matrix: Pointer to memory that contains the symmetric node-travelling cost matrix
@param  numNodes: Number of travelling-nodes in the problem
@param  population: Number of the nodes permutation (possible solution) found at each round
@param  bestNum: Number of best elements that will produce the next generation
*/
void rank_generation(int *generation_cost, int *&generation, int *&generation_copy, int *cost_matrix, int numNodes, int population, int bestNum, int numThreads){
    int i,j,source,destination,*generation_rank;

    chrono::high_resolution_clock::time_point t_start, t_end;
    chrono::duration<double> exec_time;

    generation_rank = new int[population];
            
    t_start = chrono::high_resolution_clock::now();

    // COST VECTOR COMPUTATION & RANK INITIALISATION
    fill(generation_cost, generation_cost+population, 0);
    // (tests showed that the overhead of handle parallelisation here outweights the benefits even for big matrices 100000x1000)
//#pragma omp parallel for num_threads(numThreads) private(source,destination,i,j) schedule(static)
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

    t_end = chrono::high_resolution_clock::now();
    exec_time=t_end-t_start;
    #ifdef PRINTSCOST
        printf("\t\tinitialisation & paths costs computation: %f\n",exec_time.count());
    #endif

    t_start = chrono::high_resolution_clock::now();
    sort_vector(generation_rank, generation_cost, population, numThreads);
    t_end = chrono::high_resolution_clock::now();
    exec_time=t_end-t_start;
    #ifdef PRINTSCOST
        printf("\t\tsorting: %f\n",exec_time.count());
    #endif

    //MOVE BEST ROWS TO TOP
    t_start = chrono::high_resolution_clock::now();
    move_top(generation_rank, generation, generation_copy, numNodes, bestNum);
    t_end = chrono::high_resolution_clock::now();
    exec_time=t_end-t_start;
    #ifdef PRINTSCOST
        printf("\t\tmatrix rearranging: %f\n",exec_time.count());
    #endif

    delete generation_rank;
    return;
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
@param  bestNum: Number of best elements (parents) that will produce the next generation
@param  numNodes: Number of travelling-nodes in the problem
@param  mutatProb: probability [0-1] of mutation occurence in the newly generated population element
*/
void generate(int *generation, int population, int bestNum, int numNodes, int probCentile, int numThreads){
    int i,parent1,parent2,son;

    // fill from bestnum until all population is reached
#pragma omp parallel for num_threads(numThreads) private(parent1,parent2,son,i) schedule(static)
    for(i=0; i<population-bestNum; ++i){
        if (i<bestNum) // each best must generate at least one son
            parent1 = i;          
        else
            parent1 = rand()%bestNum;

        do {    // two different parents
            parent2 = rand()%bestNum;
        } while(parent2==i);
        
        son = (bestNum+i)*numNodes;

        crossover_firstHalf_withMutation(generation, parent1, parent2, son, numNodes, probCentile);    
    }
}

bool equal_permutations(int *first, int *second, int numNodes){
    for(int i=0; i<numNodes; ++i){
        if(first[i] != second[i])
            return false;
    }
    return true;
}

void minimumCost(int *in, int *out, int *len, MPI_Datatype *dtype){
    if(in[*len-1] < out[*len-1]){
        for (int i=0; i<*len; ++i){
            out[i]=in[i];
        }
    }
}

void transferReceive_bests_allReduce(int *generation, int *generation_cost, int numNodes, int bestNum){
    int buff_size,*send_buff,*recv_buff;
    MPI_Op op;

    buff_size = numNodes+1;
    send_buff = new int[buff_size];
    recv_buff = new int[buff_size];

    copy(generation, generation+numNodes, send_buff);
    send_buff[numNodes] = generation_cost[0];

    MPI_Op_create((MPI_User_function *)minimumCost, 1, &op);

    MPI_Allreduce(send_buff, recv_buff, buff_size, MPI_INT, op, MPI_COMM_WORLD);

    if (!equal_permutations(generation, recv_buff, numNodes)){
        copy(recv_buff, recv_buff+numNodes, generation+(bestNum-1)*numNodes);
        generation_cost[bestNum-1] = recv_buff[numNodes];
    }

    return;
}