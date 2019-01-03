//////////////////// 1ST VERSION: NEW ALLOCATION WITH COPY ////////////////////////
void move_top2(int *generation_rank, int *generation, int best_num, int numNodes){
    int i,*start,*copy_mat;
    copy_mat = new int[best_num*numNodes];
    for(i=0; i<best_num; ++i){
        start = generation+generation_rank[i]*numNodes;
        copy(start, start+numNodes, copy_mat+i*numNodes);
    }
    copy(copy_mat, copy_mat+best_num*numNodes, generation);
}

///////////////////////// 1ST VERSION OF GENERATION: SON ALLOCATION THEN COPY /////////////////////////
int* crossover_firstHalf2(int *generation, int parent1, int parent2, int numNodes){
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

void generate2(int *generation, int population, int best_num, int numNodes, double mutatProb){
    int i,parent1,parent2,swap1,swap2,elemSwap,*son;
    // generate a son for each node from bests if enough space => new generation will double best_num at the end of the for cycle
#pragma parallel for num_threads(NUMTHREADS) private(parent1,parent2,swap1,swap2,elemSwap,son,i) schedule(static)
    for(i=0; i<population-best_num; ++i){
        if(i<best_num){
            parent1 = i;
        }
        else{
            parent1 = rand()%best_num;
        }

        do {    // two different parents
            parent2 = rand()%best_num;
        } while(parent2==i);

        son = crossover_firstHalf2(generation, parent1, parent2, numNodes);
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
    delete son;
}
///////////////////////////////////// END ///////////////////////////////////