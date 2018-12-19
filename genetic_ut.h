#include <set>
#include <cmath>        // rand
#include <algorithm>    // random_shuffle, copy, fill

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
    
    // COST VECTOR COMPUTATION
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
    }

    //RANK INITIALISATION
    for(i=0;i<population;++i){
        generation_rank[i]=i;
    }
    sort_vector(generation_rank, generation_cost, population);
    printMatrix(generation_cost,1,population);
    return;
}

void move_top2(int *generation_rank, int *generation, int best_num, int numNodes){
    int i,*start,*copy_mat;
    copy_mat = new int[best_num*numNodes];
    for(i=0; i<best_num; ++i){
        start = generation+generation_rank[i]*numNodes;
        copy(start, start+numNodes, copy_mat+i*numNodes);
    }
    copy(copy_mat, copy_mat+best_num*numNodes, generation);
}

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
    int i,parent1,parent2,swap1,swap2,elemSwap,*son;

    // generate a son for each node from bests if enough space => new generation will double best_num at the end of the for cycle
    for(i=0; i<best_num || best_num+i==population; ++i){
        do {    // two different parents
            parent2 = rand()%best_num;
        } while(parent2==i);

        son = crossover_firstHalf(generation, i, parent2, numNodes);
        // MUTATION
        if((rand()%100+1)<=(mutatProb*100)){
            swap1=rand()%numNodes;
            do {
                swap2=rand()%numNodes;
            } while(swap2==swap1);
            elemSwap = son[swap1];
            son[swap1] = son[swap2];
            son[swap2] = elemSwap;
        }
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
        // MUTATION
        if((rand()%100+1)<=(mutatProb*100)){
            swap1=rand()%numNodes;
            do {
                swap2=rand()%numNodes;
            } while(swap2==swap1);
            elemSwap = son[swap1];
            son[swap1] = son[swap2];
            son[swap2] = elemSwap;
        }
        copy(son, son+numNodes, generation+(i*numNodes));

        ++i;
    }
}
