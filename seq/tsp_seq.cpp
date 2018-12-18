#include "../utilities.h"

int myRand(int i){
    return rand()%i;
}

void sort_vector(int *generation_rank, int *generation_cost, int population){
    int i,j,key,key_idx; 
    for (i=1; i<population; ++i){ 
        key = generation_cost[i];
        key_idx = generation_rank[i];
        j = i-1; 
        while (j>=0 && generation_cost[j]>key){ 
            generation_cost[j+1] = generation_cost[j];
            generation_rank[j+1] = generation_rank[j];
            j = j-1; 
        } 
        generation_cost[j+1] = key;
        generation_rank[j+1] = key_idx;
    }
}

void rank_generation(int *generation_rank, int *generation, int *cost_matrix, int numNodes, int population, int best_num){
    int i,j,source,destination,*generation_cost;

    generation_cost = new int[population];
    fill(generation_cost,generation_cost+population,0);
    /*for (i=0; i< population; ++i){
        generation_cost[i] = 0;  // WHY DOES IT NEED INITIALISATION????????
    }*/
    // COST VECTOR COMPUTATION
    for(i=0; i<population; ++i){
        for(j=0; j<numNodes-1; ++j){
            source = generation[i*numNodes+j];
            destination = generation[i*numNodes+j+1];
            generation_cost[i] += cost_matrix[source*numNodes+destination];
        }
    }

    //RANK INITIALISATION
    for(i=0;i<population;++i){
        generation_rank[i]=i;
    }
    sort_vector(generation_rank, generation_cost, population);
}

void move_top(int *generation_rank, int *generation, int best_num, int numNodes){
    int i,*start,*copy_mat;
    copy_mat = new int[best_num*numNodes];
    for(i=0; i<best_num; ++i){
        start = generation+generation_rank[i]*numNodes;
        copy(start, start+numNodes, copy_mat+i*numNodes);
    }
    copy(copy_mat, copy_mat+best_num*numNodes, generation);
}

void generate(int *generation, int population, int best_num, int numNodes, double mutatProb){
    
}

void genetic_tsp(int *cost_matrix, int numNodes, int population, int best_num, int maxIt, double mutatProb){
    int *generation, *generation_cost, *generation_rank;

    generation = new int[population*numNodes];
    for (int i=0; i<population; ++i){
        for (int j=0; j<numNodes; ++j){
            generation[i*numNodes+j] = j;
        }
    }
    // RANDOM SHUFFLE
    srand(time(NULL));
    for (int i=0; i<population; ++i){
        random_shuffle(generation+i*numNodes, generation+(i+1)*numNodes, myRand);
    }
    
    // FIRST RANKING
    generation_rank = new int[population];
    rank_generation(generation_rank, generation, cost_matrix, numNodes, population, best_num);

    //MOVE BEST ROWS TO TOP
    move_top(generation_rank, generation, best_num, numNodes);

    // GENERATION ITERATION 
    for(int i=0; i<maxIt; ++i){

        // GENERATE NEW POPULATION WITH MUTATION
        generate(generation, population, best_num, numNodes, mutatProb);
        // RANKING

        // TEST EARLY STOP
    }
    return;
}


int main(int argc, const char* argv[]){
    if (argc<7){
        cerr << "need 7 args\n";
        return 1;
    }

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


    cost_matrix = new int[numNodes*numNodes];
    readHeatMat(cost_matrix, input_f, numNodes);

    best_num = (int)population*top;

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