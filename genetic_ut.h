#include <set>
#include <cmath>        // rand
#include <algorithm>    // random_shuffle, copy, fill

#define NUMTHREADS 4

int myRand(int i){
    return rand()%i;
}

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

void rank_generation(int *generation_rank, int *generation_cost, int *generation, int *cost_matrix, int numNodes, int population, int best_num){
    int i,j,source,destination;
    
    // COST VECTOR COMPUTATION & RANK INITIALISATION 
    // (tests showed that the overhead of handle parallelisation here cancel the benefits)
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
    printMatrix(generation_cost,1,population);
    return;
}

///////////////////////// 2ND VERSION: MANTAIN COPY WITH SWAP ////////////////////////////
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

///////////////////////// 2ND VERSION OF GENERATE: DIRECT WRITES ON GENERATION MATRIX /////////////////////////
void crossover_firstHalf_withMutation(int *generation, int parent1, int parent2, int son, int numNodes, double mutatProb){
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
    if((rand()%100+1)<=(mutatProb*100)){
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

void generate(int *generation, int population, int best_num, int numNodes, double mutatProb){
    int i,parent1,parent2,son;

    // fill from bestnum until all population is reached
#pragma omp parallel for num_threads(NUMTHREADS) private(parent1,parent2,son,i) schedule(static)
    for(i=0; i<population-best_num; ++i){
        if (i<best_num){  // each best must generate at least one son
            parent1 = i;          
        }
        else{
            parent1 = rand()%best_num;
        }

        do {    // two different parents
            parent2 = rand()%best_num;
        } while(parent2==i);
        
        son = (best_num+i)*numNodes;

        crossover_firstHalf_withMutation(generation, parent1, parent2, son, numNodes, mutatProb);    
    }
}
///////////////////////////////////// END ///////////////////////////////////