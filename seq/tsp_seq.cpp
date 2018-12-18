#include <chrono>
#include <ctime>

#include "../in_out.h"
#include "../genetic_ut.h"


void genetic_tsp(int *cost_matrix, int numNodes, int population, int best_num, int maxIt, double mutatProb){
    int *generation, *generation_cost, *generation_rank;

    generation = new int[population*numNodes];
    for (int i=0; i<population; ++i){
        for (int j=0; j<numNodes; ++j){
            generation[i*numNodes+j] = j;
        }
    }
    // RANDOM SHUFFLE
    for (int i=0; i<population; ++i){
        random_shuffle(generation+i*numNodes, generation+(i+1)*numNodes, myRand);
    }
    
    // FIRST RANKING
    generation_rank = new int[population];
    rank_generation(generation_rank, generation, cost_matrix, numNodes, population, best_num);

    //MOVE BEST ROWS TO TOP
    move_top(generation_rank, generation, best_num, numNodes);

    if (population==best_num){
        cout<<"Cannot generate anymore: no space in the population for new generations"<<endl;
        return;
    }

    // GENERATION ITERATION 
    for(int i=0; i<maxIt; ++i){

        // GENERATE NEW POPULATION WITH MUTATION
        generate(generation, population, best_num, numNodes, mutatProb);
        // RANKING
        rank_generation(generation_rank, generation, cost_matrix, numNodes, population, best_num);
        //MOVE BEST ROWS TO TOP
        move_top(generation_rank, generation, best_num, numNodes);
        //printMatrix(generation,population,numNodes);

        // TEST EARLY STOP
    }
    return;
}


int main(int argc, const char* argv[]){
    if (argc<7){
        cerr << "need 7 args\n";
        return 1;
    }

    srand(time(NULL));

    chrono::high_resolution_clock::time_point t_start,t_end;
    chrono::duration<double> exec_time;

    int numNodes,population,best_num,maxIt,*cost_matrix;
    double mutatProb,top;
    const char *input_f;

    numNodes = atoi(argv[1]);
    population = atoi(argv[2]);
    top = atof(argv[3]);
    maxIt = atoi(argv[4]);
    mutatProb = atof(argv[5]);
    input_f = argv[6];

    best_num = (int)population*top;

    if (population<best_num){
        cerr << "Selection greater than population size"<< endl;
    }


    cost_matrix = new int[numNodes*numNodes];
    readHeatMat(cost_matrix, input_f, numNodes);

    /////////////////////////////////////////////
    t_start = chrono::high_resolution_clock::now();
    /////////////////////////////////////////////
    genetic_tsp(cost_matrix, numNodes, population, best_num, maxIt, mutatProb);
    /////////////////////////////////////////////
    t_end = chrono::high_resolution_clock::now();
    /////////////////////////////////////////////

    exec_time = t_end - t_start;
    //printMatrix(cost_matrix);
    //printf("%f\n",exec_time.count());

    return 0;   
}