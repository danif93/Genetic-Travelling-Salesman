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

/////////////////////////// EXCHANGE BETWEEN TWO NODES //////////////////////////
void transferReceive_bests_between2(int* generation, int* generation_cost, int numNodes, int bestNum, int me, int numInstances, int messageNum){
    int sendTo, recvFrom;

    sendTo = (me + messageNum) % numInstances;
    if (sendTo == me)
        return;
    recvFrom = (me - messageNum) % numInstances;
    if(recvFrom<0)
        recvFrom += numInstances;

    int position,buff_size,recv_cost;
    char *send_buff, *recv_buff;
    MPI_Request request;
    MPI_Status status;

    buff_size = (numNodes+1)*sizeof(int);
    send_buff = new char[buff_size];
    position = 0;

    MPI_Pack(&generation_cost[0], 1, MPI_INT, send_buff, buff_size, &position, MPI_COMM_WORLD);
    MPI_Pack(generation, numNodes, MPI_INT, send_buff, buff_size, &position, MPI_COMM_WORLD);
    MPI_Isend(send_buff, position, MPI_PACKED, sendTo, 0, MPI_COMM_WORLD,&request);

    recv_buff = new char[buff_size];
    position = 0;

    MPI_Recv(recv_buff, buff_size, MPI_PACKED, recvFrom, 0, MPI_COMM_WORLD, &status);
    MPI_Unpack(recv_buff, buff_size, &position, &recv_cost, 1, MPI_INT, MPI_COMM_WORLD); 

    if (recv_cost < generation_cost[bestNum-1]){
        MPI_Unpack(recv_buff, buff_size, &position, generation+(bestNum-1)*numNodes, numNodes, MPI_INT, MPI_COMM_WORLD);
        generation_cost[bestNum-1] = recv_cost;
    }

    delete send_buff;
    delete recv_buff;
    return;
}

/////////////////////////// EXCHANGE BETWEEN ALL THE NODES IN A BARRIER-LIKE PROCEDURE //////////////////////////
void transferReceive_bests_barrier(int *generation, int *generation_cost, int numNodes, int bestNum, int me, int numInstances){
    int i,position,buff_size,cost,recv_cost,sendTo,recvFrom,*permutation;
    char *send_buff, *recv_buff;
    MPI_Request request;
    MPI_Status status;

    buff_size = (numNodes+1)*sizeof(int);
    send_buff = new char[buff_size];
    recv_buff = new char[buff_size];
    permutation = new int[numNodes];

    copy(generation, generation+numNodes, permutation);
    cost = generation_cost[0];

    for(i=1; i<numInstances; i=i<<1){
        sendTo = (me+i)%numInstances;
        position = 0;
        MPI_Pack(&cost, 1, MPI_INT, send_buff, buff_size, &position, MPI_COMM_WORLD);
        MPI_Pack(permutation, numNodes, MPI_INT, send_buff, buff_size, &position, MPI_COMM_WORLD);
        MPI_Isend(send_buff, position, MPI_PACKED, sendTo, 0, MPI_COMM_WORLD,&request);

        recvFrom = me-i;
        if(recvFrom<0)
            recvFrom += numInstances;
        position = 0;
        MPI_Recv(recv_buff, buff_size, MPI_PACKED, recvFrom, 0, MPI_COMM_WORLD, &status);
        MPI_Unpack(recv_buff, buff_size, &position, &recv_cost, 1, MPI_INT, MPI_COMM_WORLD); 

        if (recv_cost < cost){
            cost = recv_cost;
            MPI_Unpack(recv_buff, buff_size, &position, permutation, numNodes, MPI_INT, MPI_COMM_WORLD);
        }
    }

    if ((cost <= generation_cost[0]) /*&& !equal_permutations(generation,permutation, numNodes)*/){
        copy(permutation, permutation+numNodes, generation+(bestNum-1)*numNodes);
        generation_cost[bestNum-1] = cost;
    }
    return;
}