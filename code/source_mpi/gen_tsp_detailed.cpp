/**
gen_tsp.cpp
Purpose: Genetic alghorithm approach for the travelling salesman problem

@author Danilo Franco
*/

#include <chrono>
#include <ctime>
#include "mpi.h"

#include "../in_out.h"
#include "../genetic_utils_detailed.h"
#include "../other_funcs.h"

#define AVGELEMS 10  //number of elements from which the average for early-stopping is computed
#define TRANSFERRATE 10
#define DETAILEDCOSTS

FILE *generationFile, *transferFile;

/**
Finds and returns the solution for the tsp

@param  numThreads: Number of processing elements are due to work on each parallel section
@param  me: Index of the current executing node in the cluster
@param  numInstances: Amount of nodes currently working on finding the solution
@param  cost_matrix: Pointer to memory that contains the symmetric node-travelling cost matrix 
@param  numNodes: Number of travelling-nodes in the problem
@param  population: Number of the nodes permutation (possible solution) found at each round
@param  top: percentage [0-1] of elements from population that are going to generate new permutation
@param  maxIt: number of max generation rounds
@param  mutatProb: probability [0-1] of mutation occurrence in the newly generated population element
@param  earlyStopRounds: number of latest iterations from which the average of best AVGELEMS must be computed 
            in order to establish convergence
@param  earlyStopParams: Comparison parameter for early stopping

@return     Pointer to the found nodes permutation (integer index) + solution cost + convergence boolean
*/
int* genetic_tsp(int me, int numInstances, int numThreads, int *cost_matrix, int numNodes, int population, double top, int maxIt, double mutatProb, int earlyStopRounds, double earlyStopParam){
    int i, j, best_num, probCentile, sendTo, recvFrom, *generation, *generation_copy, *generation_cost, *solution;
    double avg, *lastRounds;
    chrono::high_resolution_clock::time_point t_start, t_end;
    chrono::duration<double> exec_time;

    best_num = population*top;
    probCentile = mutatProb*100;
    
    lastRounds = new double[earlyStopRounds];
    solution = new int[numNodes+2];
    generation = new int[population*numNodes];
    generation_copy = new int[population*numNodes];
    generation_cost = new int[population];

    // SEQUENTIAL INITIALISATION && RANDOM SHUFFLE (over a single row)
    for (i=0; i<population; ++i){
        for (j=0; j<numNodes; ++j)
            generation[i*numNodes+j] = j;
        random_shuffle(generation+i*numNodes, generation+(i+1)*numNodes, myRand);
    }
    
    // FIRST RANKING
    rank_generation(generation_cost, generation, generation_copy, cost_matrix, numNodes, population, best_num, numThreads);

    if (population==best_num){
        copy(generation, generation+numNodes, solution);
        solution[numNodes] = generation_cost[0];
        solution[numNodes+1] = 0;
        return solution;
    }

    // GENERATION ITERATION 
    for(i=1; i<=maxIt; ++i){

        solution[numNodes+1] = 0;

        // GENERATE NEW POPULATION WITH MUTATION
        t_start = chrono::high_resolution_clock::now();
        generate(generation, population, best_num, numNodes, probCentile, numThreads);
        t_end = chrono::high_resolution_clock::now();
        exec_time=t_end-t_start;
#ifdef DETAILEDCOSTS
        fprintf(generationFile,"%d %d %d %f\n",numNodes,population,best_num,exec_time.count());
#endif

        // RANKING
        t_start = chrono::high_resolution_clock::now();
        rank_generation(generation_cost, generation, generation_copy, cost_matrix, numNodes, population, best_num, numThreads);
        t_end = chrono::high_resolution_clock::now();
        exec_time = t_end-t_start;

        // compute average of best #AVGELEMS costs
        avg = 0;
        for(j=0; j<AVGELEMS; ++j){
            avg += generation_cost[j];
        }
        lastRounds[(i-1)%earlyStopRounds] = avg/AVGELEMS;

        // EXCHANGE BEST WITH OTHER NODES
        if(numInstances>1 && i!=maxIt && !(i%TRANSFERRATE)){    
            t_start = chrono::high_resolution_clock::now();
            transferReceive_bests_allReduce(generation, generation_cost, numNodes, best_num);
            t_end = chrono::high_resolution_clock::now();
            exec_time = t_end-t_start;
#ifdef DETAILEDCOSTS
            fprintf(transferFile,"%d %d %d %f\n",numNodes,population,best_num,exec_time.count());
#endif
            continue;
        }

        // TEST EARLY STOP (with short-circuit to ensure that lastRounds is filled before computing the stdDev over it)
        if(i>=earlyStopRounds && stdDev(lastRounds, earlyStopRounds)<=earlyStopParam){
            // move to next exchange session (hoping that can help moving out from a fake convergence)
            // ... moreover other nodes might continue to expect messages
            if(i<maxIt-TRANSFERRATE){
                i += TRANSFERRATE-(i%TRANSFERRATE)-1;
            }
            solution[numNodes+1] = 1;
        }
    }

    copy(generation, generation+numNodes, solution);
    solution[numNodes] = generation_cost[0];
  
    delete lastRounds;
    delete generation;
    delete generation_copy;
    delete generation_cost;

    return solution;
}

int main(int argc, char *argv[]){
    if (argc<10){
        cerr << "need 9 args\n";
        return 1;
    }

    int me,numInstances,numThreads,numNodes,population,best_num,maxIt,earlyStopRounds,earlyStopParam,*cost_matrix,*solution;
    double mutatProb,top;
    const char *input_f;
    chrono::high_resolution_clock::time_point t_start,t_end;
    chrono::duration<double> exec_time;

    numThreads = atoi(argv[1]);
    numNodes = atoi(argv[2]);
    population = atoi(argv[3]);
    top = atof(argv[4]);
    maxIt = atoi(argv[5]);
    mutatProb = atof(argv[6]);
    earlyStopRounds = atoi(argv[7]);
    earlyStopParam = atof(argv[8]);
    input_f = argv[9];

    if (numThreads<1 ||
        top<0 || top>1 ||                               // selection percentage from total population
        population < AVGELEMS ||                        // for early stop averaging purposes
        numNodes <= 1 ||                                // graph with at least 2 nodes
        maxIt <0 || 
        mutatProb<0 || mutatProb>1 ||                   // probability!
        earlyStopRounds>maxIt || earlyStopRounds<=0 ||  // latest runs influence
        earlyStopParam<0){                              // standard deviation!
        cerr <<"Invalid arguments!"<< endl;
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &numInstances);
    
    srand(time(NULL)+me);

    // in order to see convergence if in the last message exchange a node receives a good permutation
    if(earlyStopRounds>TRANSFERRATE){
        earlyStopRounds = TRANSFERRATE;
    }

    generationFile = fopen(("proj_HPC/code/results/detailed/parallelMPI/generation_"+to_string(me)+".txt").c_str(), "a");
    pathComputationFile = fopen(("proj_HPC/code/results/detailed/parallelMPI/path_"+to_string(me)+".txt").c_str(), "a");
    sortingFile = fopen(("proj_HPC/code/results/detailed/parallelMPI/sort_"+to_string(me)+".txt").c_str(), "a");
    rearrangeFile = fopen(("proj_HPC/code/results/detailed/parallelMPI/rearrange_"+to_string(me)+".txt").c_str(), "a");
    transferFile = fopen(("proj_HPC/code/results/detailed/parallelMPI/transfer_"+to_string(me)+".txt").c_str(), "a");

    cost_matrix = new int[numNodes*numNodes];
    readHeatMat(cost_matrix, input_f, numNodes);

    t_start = chrono::high_resolution_clock::now();
    solution = genetic_tsp(me, numInstances, numThreads, cost_matrix, numNodes, population, top, maxIt, mutatProb, earlyStopRounds, earlyStopParam);
    t_end = chrono::high_resolution_clock::now();
    exec_time = t_end - t_start;

    MPI_Finalize();

    fclose(generationFile);
    fclose(pathComputationFile);
    fclose(sortingFile);
    fclose(rearrangeFile);
    fclose(transferFile);

    delete cost_matrix;
    delete solution;

    return 0;   
}