/**
gen_tsp.cpp
Purpose: Genetic alghorithm approach for the travelling salesman problem

@author Danilo Franco
*/

#include <chrono>
#include <ctime>
#include <string>
#include "mpi.h"

#include "../in_out.h"
#include "../genetic_utils.h"
#include "../other_funcs.h"

#define AVGELEMS 5      //number of elements from which the average for early-stopping is computed
//#define PRINTSCOST    // detailed time prints of each phase
//#define PRINTSMAT     // print population matrix and relative cost at each iteration
#define PRINTSGRAPH     // print the final computational cost with the setting, its minimum solution cost and convergence boolean

/**
Finds and returns the solution for the tsp

@param  numThreads: Number of processing elements are due to work on each parallel section
@param  cost_matrix: Pointer to memory that contains the symmetric node-travelling cost matrix 
@param  numNodes: Number of travelling-nodes in the problem
@param  population: Number of the nodes permutation (possible solution) found at each round
@param  top: percentage [0-1] of elements from population that are going to generate new permutation
@param  maxIt: number of max generation rounds
@param  mutatProb: probability [0-1] of mutation occurrence in the newly generated population element
@param  earlyStopRounds: number of latest iterations from which the average of best AVGELEMS must be computed 
            in order to establish convergence
@param  earlyStopParam: Comparison parameter for early stopping

@return     Pointer to the found nodes permutation (integer index) + solution cost + convergence boolean
*/
int* genetic_tsp(int numThreads, int *cost_matrix, int numNodes, int population, double top, int maxIt, double mutatProb, int earlyStopRounds, double earlyStopParam){
    int countIt, i, j, best_num, probCentile, sendTo, recvFrom, *generation, *generation_copy, *generation_cost, *solution;
    double avg, *lastRounds;
    chrono::high_resolution_clock::time_point t_start, t_end;
    chrono::duration<double> exec_time;

    countIt = 0;
    best_num = population*top;
    probCentile = mutatProb*100;
    
    lastRounds = new double[earlyStopRounds];
    solution = new int[numNodes+3];
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

    solution[numNodes+1] = 0; //not converged

    if (population==best_num){
#ifdef PRINTSCOST
        printf("Cannot generate anymore: no space in the population for new generations\n");
#endif
        copy(generation, generation+numNodes, solution);
        solution[numNodes] = generation_cost[0];
        solution[numNodes+2] = countIt;
        return solution;
    }

    // GENERATION ITERATION 
    for(i=1; i<=maxIt; ++i){
#if defined(PRINTSCOST) || defined(PRINTSMAT)
        printf("#%d\n",i);
#endif

#ifdef PRINTSMAT
        printMatrix(generation,population,numNodes);
        printMatrix(generation_cost,1,population);
#endif

        ++countIt;
        
        // GENERATE NEW POPULATION WITH MUTATION
        t_start = chrono::high_resolution_clock::now();
        generate(generation, population, best_num, numNodes, probCentile, numThreads);
        t_end = chrono::high_resolution_clock::now();
        exec_time=t_end-t_start;
#ifdef PRINTSCOST
        printf("\tgeneration: %f\n\t-------------\n",exec_time.count());
#endif

        // RANKING
        t_start = chrono::high_resolution_clock::now();
        rank_generation(generation_cost, generation, generation_copy, cost_matrix, numNodes, population, best_num, numThreads);
        t_end = chrono::high_resolution_clock::now();
        exec_time = t_end-t_start;
#ifdef PRINTSCOST
        printf("\tranking: %f\n\t-------------\n",exec_time.count());
#endif

        // compute average of best #AVGELEMS costs
        avg = 0;
        for(j=0; j<AVGELEMS; ++j){
            avg += generation_cost[j];
        }
        lastRounds[(i-1)%earlyStopRounds] = avg/AVGELEMS;
#ifdef PRINTSCOST
        printf("\tbest %d average travelling cost: %f\n",AVGELEMS,lastRounds[(i-1)%earlyStopRounds]);
        printf("\tbest %d standard deviation: %f\n\t-------------\n",AVGELEMS,stdDev(lastRounds, earlyStopRounds));
#endif

        // TEST EARLY STOP (with short-circuit to ensure that lastRounds is filled before computing the stdDev over it)
        if(i>=earlyStopRounds && stdDev(lastRounds, earlyStopRounds)<=earlyStopParam){
#ifdef PRINTSCOST
            printf("\n\t\tEarly stop!\n\n");
#endif
            solution[numNodes+1] = 1;
            break;
        }
    }

    copy(generation, generation+numNodes, solution);
    solution[numNodes] = generation_cost[0];
    solution[numNodes+2] = countIt;
        
    delete lastRounds;
    delete generation;
    delete generation_copy;
    delete generation_cost;

    return solution;
}

int main(int argc, char *argv[]){
    if (argc<9){
        cerr << "need 9 args\n";
        return 1;
    }

    int me,numThreads,numNodes,population,best_num,maxIt,earlyStopRounds,earlyStopParam,*cost_matrix,*solution;
    double mutatProb,top;
    FILE *pFile;
    const char *input_f;
    string outDir;
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
        maxIt < 0 || 
        mutatProb<0 || mutatProb>1 ||                   // probability!
        earlyStopRounds>maxIt || earlyStopRounds<=0 ||  // latest runs influence
        earlyStopParam<0){                              // standard deviation!
        cerr <<"Invalid arguments!"<< endl;
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    srand(time(NULL)+me);

    if(numThreads==1){
        outDir = string("proj_HPC/code/results/total/phase2/sequential/");
    } else {
        outDir = string("proj_HPC/code/results/total/phase2/parallel/");
    }

    pFile = fopen((outDir+to_string(me)+".txt").c_str(), "a");

    cost_matrix = new int[numNodes*numNodes];
    readHeatMat(cost_matrix, input_f, numNodes);
#ifdef PRINTSMAT
    printMatrix(cost_matrix, numNodes, numNodes);
#endif

    t_start = chrono::high_resolution_clock::now();
    solution = genetic_tsp(numThreads, cost_matrix, numNodes, population, top, maxIt, mutatProb, earlyStopRounds, earlyStopParam);
    t_end = chrono::high_resolution_clock::now();
    exec_time = t_end - t_start;

#ifdef PRINTSCOST
    printf("\nTotal execution cost: %f\n\n",exec_time.count());
#endif

#ifdef PRINTSMAT
    printMatrix(solution, 1, numNodes);
#endif

#ifdef PRINTSGRAPH
    fprintf(pFile,"%d %d %d %f %d %d %d\n",numNodes,population,int(population*top),exec_time.count(),solution[numNodes],solution[numNodes+1],solution[numNodes+2]);
#endif

    MPI_Finalize();
    fclose(pFile);

    delete cost_matrix;
    delete solution;

    return 0;   
}