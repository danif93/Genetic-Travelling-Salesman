#include <set>
#include <cmath>        // rand
#include <algorithm>    // random_shuffle, copy, fill

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
    //printMatrix(generation_cost,1,population);
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

int* crossover_firstHalf(int *generation, int parent1, int parent2, int numNodes){
    set<int> nodes;
    int j,k,half,elem,*son;

    half = floor(numNodes/2);
    son = new int[numNodes];

    // take first half from parent1
    for(j=0; j<half; ++j){
        son[j]=generation[parent1*numNodes+j];
        nodes.insert(son[j]);
    }
    // add the remaining elements from parent2
    for(k=0; k<numNodes; ++k){
        elem = generation[parent2*numNodes+k];
        if(nodes.find(elem)==nodes.end()){
            son[j] = elem;
            ++j;
        }
    }
    return son;
}

void generate(int *generation, int population, int best_num, int numNodes, double mutatProb){
    int i,parent1,parent2,*son;

    // generate a son for each node from bests if enough space => new generation will double best_num at the end of the for cycle
    for(i=0; i<best_num || best_num+i==population; ++i){
        do {    // two different parents
            parent2 = rand()%best_num;
        } while(parent2==i);

        son = crossover_firstHalf(generation, i, parent2, numNodes);
        // MUTATION MISSING
        copy(son, son+numNodes, generation+((best_num+i)*numNodes));
    }

    i += best_num;
    // fill until population number
    while(i<population){
        parent1 = rand()%best_num;
        do {    // two different parents
            parent2 = rand()%best_num;
        } while(parent2==parent1);

        son = crossover_firstHalf(generation, parent1, parent2, numNodes);
        // MUTATION MISSING
        copy(son, son+numNodes, generation+(i*numNodes));

        ++i;
    }
}
