#include <chrono>
#include <ctime>

#include "in_out.h"
#include "genetic_ut.h"
#include "other_func.h"

#define AVGELEMS 3

int* genetic_tsp(int *cost_matrix, int numNodes, int population, int best_num, int maxIt, double mutatProb, int earlyStopRounds, double earlyStopParam){
    int i, j, *generation, *generation_copy, *generation_rank, *generation_cost, *solution;
    double avg, *lastRounds;
    chrono::high_resolution_clock::time_point t_start, t_end;
    chrono::duration<double> exec_time;
    
    lastRounds = new double[earlyStopRounds];
    solution = new int[numNodes];
    generation_copy = new int[population*numNodes];
    generation = new int[population*numNodes];
    for (i=0; i<population; ++i){
        for (int j=0; j<numNodes; ++j){
            generation[i*numNodes+j] = j;
        }
    }
    // RANDOM SHUFFLE
    t_start = chrono::high_resolution_clock::now();
    for (i=0; i<population; ++i){
        random_shuffle(generation+i*numNodes, generation+(i+1)*numNodes, myRand);
    }
    t_end = chrono::high_resolution_clock::now();
    exec_time=t_end-t_start;
    //printf("Random shuffle: %f\n",exec_time.count());
    
    // FIRST RANKING
    generation_rank = new int[population];
    generation_cost = new int[population];
    t_start = chrono::high_resolution_clock::now();
    fill(generation_cost, generation_cost+population, 0);
    rank_generation(generation_rank, generation_cost, generation, cost_matrix, numNodes, population, best_num);
    t_end = chrono::high_resolution_clock::now();
    exec_time=t_end-t_start;
    //printf("first ranking: %f\n",exec_time.count());

    //MOVE BEST ROWS TO TOP
    move_top(generation_rank, generation, generation_copy, numNodes, best_num);

    if (population==best_num){
        printf("Cannot generate anymore: no space in the population for new generations\n");
        copy(generation, generation+numNodes, solution);
        return solution;
    }

    // GENERATION ITERATION 
    for(i=0; i<maxIt; ++i){
        //printf("Iteration %d\n", i+1);
        // GENERATE NEW POPULATION WITH MUTATION
        t_start = chrono::high_resolution_clock::now();
        generate(generation, population, best_num, numNodes, mutatProb);
        t_end = chrono::high_resolution_clock::now();
        exec_time=t_end-t_start;
        //printf("%d generation: %f\n",i+1,exec_time.count());

        // RANKING
        t_start = chrono::high_resolution_clock::now();
        fill(generation_cost, generation_cost+population, 0);
        rank_generation(generation_rank, generation_cost, generation, cost_matrix, numNodes, population, best_num);
        t_end = chrono::high_resolution_clock::now();
        exec_time=t_end-t_start;
        //printf("%d ranking: %f\n",i+1,exec_time.count());

        // compute average of best #AVGELEMS costs
        avg = 0;
        for(j=0; j<AVGELEMS; ++j){
            avg += generation_cost[j];
        }
        lastRounds[i%earlyStopRounds]= avg/AVGELEMS;
        
        //MOVE BEST ROWS TO TOP
        move_top(generation_rank, generation, generation_copy, numNodes, best_num);
        //printMatrix(generation, population, numNodes);
        
        // TEST EARLY STOP
        if(i>earlyStopRounds && stdDev(lastRounds, earlyStopRounds)<=earlyStopParam){
            //printf("Early stop!\n\n");
            break;
        }
    }

    copy(generation, generation+numNodes, solution);
        
    delete lastRounds;
    delete generation;
    delete generation_copy;
    delete generation_rank;
    delete generation_cost;

    printf("Ended at iteration: %d\n",i);
    return solution;
}


int main(int argc, const char* argv[]){
    if (argc<8){
        cerr << "need 8 args\n";
        return 1;
    }

    srand(time(NULL));

    chrono::high_resolution_clock::time_point t_start,t_end;
    chrono::duration<double> exec_time;

    int numNodes,population,best_num,maxIt,earlyStopRounds,earlyStopParam,*cost_matrix,*solution;
    double mutatProb,top;
    const char *input_f;

    numNodes = atoi(argv[1]);
    population = atoi(argv[2]);
    top = atof(argv[3]);
    maxIt = atoi(argv[4]);
    mutatProb = atof(argv[5]);
    earlyStopRounds = atoi(argv[6]);
    earlyStopParam = atof(argv[7]);
    input_f = argv[8];

    best_num = (int)population*top;

    if (population<best_num){
        cerr << "Selection greater than population size"<< endl;
    }

    cost_matrix = new int[numNodes*numNodes];
    readHeatMat(cost_matrix, input_f, numNodes);

    /////////////////////////////////////////////
    t_start = chrono::high_resolution_clock::now();
    /////////////////////////////////////////////
    solution = genetic_tsp(cost_matrix, numNodes, population, best_num, maxIt, mutatProb, earlyStopRounds, earlyStopParam);
    /////////////////////////////////////////////
    t_end = chrono::high_resolution_clock::now();
    /////////////////////////////////////////////

    exec_time = t_end - t_start;
    printf("%f\n",exec_time.count());

    //printMatrix(cost_matrix, numNodes, numNodes);
    //printMatrix(solution, 1, numNodes);

    delete cost_matrix;
    delete solution;

    return 0;   
}