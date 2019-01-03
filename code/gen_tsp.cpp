/**
gen_tsp.cpp
Purpose: Genetic alghorithm approach for the travelling salesman problem

@author Danilo Franco
*/

#include <chrono>
#include <ctime>

#include "in_out.h"
#include "genetic_utils.h"
#include "other_funcs.h"

#define AVGELEMS 3  //number of elements from which the average for early-stopping is computed
//#define PRINTS

/**
Finds and returns the solution for the tsp

@param  cost_matrix: Pointer to memory that contains the symmetric node-travelling cost matrix 
@param  numNodes: Number of travelling-nodes in the problem
@param  population: Number of the nodes permutation (possible solution) found at each round
@param  top: percentage [0-1] of elements from population that are going to generate new permutation
@param  maxIt: number of max generation rounds
@param  mutatProb: probability [0-1] of mutation occurence in the newly generated population element
@param  earlyStopRounds: number of latest iterations from which the average of best AVGELEMS must be computed 
            in order to establish convergence
@param  earlyStopParams: Comparison parameter for early stopping

@return     Pointer to the found nodes permutation (integer index)
*/
int* genetic_tsp(int *cost_matrix, int numNodes, int population, double top, int maxIt, double mutatProb, int earlyStopRounds, double earlyStopParam){
    int i, j, best_num, probCentile, *generation, *generation_copy, *generation_rank, *generation_cost, *solution;
    double avg, *lastRounds;
    chrono::high_resolution_clock::time_point t_start, t_end;
    chrono::duration<double> exec_time;

    best_num = population*top;
    probCentile = mutatProb*100;
    
    lastRounds = new double[earlyStopRounds];
    solution = new int[numNodes];
    generation = new int[population*numNodes];
    generation_copy = new int[population*numNodes];
    generation_rank = new int[population];
    generation_cost = new int[population];

    // SEQUENTIAL INITIALISATION && RANDOM SHUFFLE (over a single row)
    for (i=0; i<population; ++i){
        for (j=0; j<numNodes; ++j)
            generation[i*numNodes+j] = j;
        random_shuffle(generation+i*numNodes, generation+(i+1)*numNodes, myRand);
    }
    
    // FIRST RANKING
    rank_generation(generation_rank, generation_cost, generation, cost_matrix, numNodes, population, best_num);

    //MOVE BEST ROWS TO TOP
    move_top(generation_rank, generation, generation_copy, numNodes, best_num);

    if (population==best_num){
#ifdef PRINTS
        printf("Cannot generate anymore: no space in the population for new generations\n");
#endif
        copy(generation, generation+numNodes, solution);
        return solution;
    }

    // GENERATION ITERATION 
    for(i=0; i<maxIt; ++i){
#ifdef PRINTS
        printf("#%d\n",i+1);
#endif

        // GENERATE NEW POPULATION WITH MUTATION
        t_start = chrono::high_resolution_clock::now();
        generate(generation, population, best_num, numNodes, probCentile);
        t_end = chrono::high_resolution_clock::now();
        exec_time=t_end-t_start;
#ifdef PRINTS
        printf("\tgeneration: %f\n",exec_time.count());
#endif

        // RANKING
        t_start = chrono::high_resolution_clock::now();
        rank_generation(generation_rank, generation_cost, generation, cost_matrix, numNodes, population, best_num);
        t_end = chrono::high_resolution_clock::now();
        exec_time=t_end-t_start;
#ifdef PRINTS
        printf("\tranking: %f\n",exec_time.count());
#endif

        // compute average of best #AVGELEMS costs
        avg = 0;
        for(j=0; j<AVGELEMS; ++j){
            avg += generation_cost[j];
        }
        lastRounds[i%earlyStopRounds]= avg/AVGELEMS;
#ifdef PRINTS
        printf("\tbest %d average travelling cost: %f\n",AVGELEMS,lastRounds[i%earlyStopRounds]);
        printf("\tbest %d standard deviation: %f\n",AVGELEMS,stdDev(lastRounds, earlyStopRounds));
#endif
        
        //MOVE BEST ROWS TO TOP
        move_top(generation_rank, generation, generation_copy, numNodes, best_num);

        // TEST EARLY STOP (with short-circuit to ensure that lastRounds is filled before computing the stdDev over it)
        if(i>earlyStopRounds && stdDev(lastRounds, earlyStopRounds)<=earlyStopParam){
#ifdef PRINTS
            printf("\n\t\tEarly stop!\n\n");
#endif
            break;
        }
    }

    copy(generation, generation+numNodes, solution);
        
    delete lastRounds;
    delete generation;
    delete generation_copy;
    delete generation_rank;
    delete generation_cost;

    return solution;
}

int main(int argc, const char* argv[]){
    if (argc<8){
        cerr << "need 8 args\n";
        return 1;
    }

    srand(time(NULL));

    int numNodes,population,best_num,maxIt,earlyStopRounds,earlyStopParam,*cost_matrix,*solution;
    double mutatProb,top;
    const char *input_f;
    chrono::high_resolution_clock::time_point t_start,t_end;
    chrono::duration<double> exec_time;

    numNodes = atoi(argv[1]);
    population = atoi(argv[2]);
    top = atof(argv[3]);
    maxIt = atoi(argv[4]);
    mutatProb = atof(argv[5]);
    earlyStopRounds = atoi(argv[6]);
    earlyStopParam = atof(argv[7]);
    input_f = argv[8];

    if (top<0 || top>1 || 
        population<=0 || 
        numNodes <=1 || 
        maxIt <0 || 
        mutatProb<0 || mutatProb>1 || 
        earlyStopRounds>maxIt || earlyStopRounds<=0 || 
        earlyStopParam<0){
        cerr <<"Invalid arguments!"<< endl;
        return 1;
    }

    cost_matrix = new int[numNodes*numNodes];
    readHeatMat(cost_matrix, input_f, numNodes);
    //printMatrix(cost_matrix, numNodes, numNodes);


    /////////////////////////////////////////////
    t_start = chrono::high_resolution_clock::now();
    /////////////////////////////////////////////
    solution = genetic_tsp(cost_matrix, numNodes, population, top, maxIt, mutatProb, earlyStopRounds, earlyStopParam);
    /////////////////////////////////////////////
    t_end = chrono::high_resolution_clock::now();
    /////////////////////////////////////////////
    exec_time = t_end - t_start;

#ifdef PRINTS
    printf("\nTotal execution cost: %f\n\n",exec_time.count());
    //printMatrix(solution, 1, numNodes);
#endif

    delete cost_matrix;
    delete solution;

    return 0;   
}