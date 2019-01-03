/////////////////////// QUICK SORT ///////////////////////
/**
Swap two integer

@param  a: pointer to the first element to be swapped
@param  b: pointer to the second element to be swapped
*/
void swap_intElem(int* a, int* b) { 
    int n = *a; 
    *a = *b; 
    *b = n; 
}

/**
This function takes the last number as the pivot and places it such that all the smaller elements are on its left 
    and the greaters on its right; since all the newborn permutation are put at the end of the generation matrix, taking
    the last element as the pivot might end up near the best scenario for quicksort (pivot=middle)

@param  generation_rank: Index array
@param  generation_cost: Sorting array
@param  low: starting sorting index
@param  high: ending sorting index

@return     pivot correct position
*/
int partition (int *generation_rank, int *generation_cost, int low, int high) { 
    int pivot,i,j;
    pivot = generation_cost[high];
    i = low-1; 
  
    for (j=low; j<=high-1; j++) { 
        //current element <= pivot: put it at the first free spot on the left
        if (generation_cost[j] <= pivot) { 
            i++;    //next free spot
            swap_intElem(&generation_cost[i], &generation_cost[j]);
            swap_intElem(&generation_rank[i], &generation_rank[j]);
        } 
    }
    //place the pivot after all the smaller elements
    swap_intElem(&generation_cost[i+1], &generation_cost[high]);
    swap_intElem(&generation_rank[i+1], &generation_rank[high]);
    return i+1; 
} 
  
/**
QuickSort: assume a pivot and place it in his correct position (smallers on its left, greaters on its right);
    repeat recursively on the two parts (smallers and greaters)

@param  generation_rank: Index array
@param  generation_cost: Sorting array
@param  low: starting sorting index
@param  high: ending sorting index
*/
void quickSort(int *generation_rank, int *generation_cost, int low, int high) { 
    if (low<high) { 
        int pIdx = partition(generation_rank, generation_cost, low, high);
        
        quickSort(generation_rank, generation_cost, low, pIdx-1);
        quickSort(generation_rank, generation_cost, pIdx+1, high);
    } 
}

/////////////////////// MERGE SORT ///////////////////////
void merge (int* generation_cost, int* generation_rank, int *temp, int low, int mid, int high) {
    int i,j,k;
    i=low;
    j=mid+1;
    k=low;
        
    while (i<=mid && j<=high) {
        if (generation_cost[i] <= generation_cost[j]){
            temp[k*2] = generation_cost[i];
            temp[k*2+1] = generation_rank[i];
            k++;i++;
        }
        else{
            temp[k*2] = generation_cost[j];
            temp[k*2+1] = generation_rank[j];
            k++;j++;
        }
    }
        
    while (i<=mid){
        temp[k*2] = generation_cost[i];
        temp[k*2+1] = generation_rank[i];
        k++;i++;
    }
                
    while (j<=high){
        temp[k*2] = generation_cost[j];
        temp[k*2+1] = generation_rank[j];
        k++;j++;
    }
                
    for (i=low; i<k; i++){
        generation_cost[i] = temp[(i)*2];
        generation_rank[i] = temp[(i)*2+1];
    }
}

void mergesort_help(int* generation_cost, int* generation_rank, int *temp, int low, int high){
    if (low < high) {                
        int mid = (low + high)/2;
        mergesort_help(generation_cost, generation_rank, temp, low, mid);
        mergesort_help(generation_cost, generation_rank, temp, mid+1, high);
        merge(generation_cost, generation_rank, temp, low, mid, high);
    }
}

void mergesort (int* generation_cost, int* generation_rank, int low, int high, int cores, int population) {
    int q,rem,start,end,kk,ll,hh,k,flag,*temp;
    temp = new int[population*2];
    
    // ############## SORT
    q = (high+1-low)/cores;   // floor quotient
    rem = (high+1-low)%cores; // remainder
    start = low;

    for (k=0; k<cores; ++k){
        if (rem){
            end = start+q;
            rem--;
        }
        else
            end = start+q-1;

        #pragma omp task
        mergesort_help(generation_cost, generation_rank, temp, start, end);
        start = end+1;
    }
    #pragma omp taskwait
    
    // ############## MERGE
    start = low;
    rem = (high+1-low)%cores;
    k = cores;
    flag = 0;
    hh = low-1;
    int idx[cores+1]; 
    idx[0]=low;

    for (kk = 0; kk<cores; ++kk){
        ll=hh+1;
        if(rem){
            hh=ll+q;
            rem--;
        }
        else
            hh=ll+q-1;

        idx[kk+1]=hh;
    }

    while (k != 1) {
        for(kk=0; kk<k-1; kk+=2) {
            #pragma omp task shared(idx)        
            merge(generation_cost, generation_rank, temp, idx[kk]+int(kk!=0), idx[kk+1], idx[kk+2]);
        }
        #pragma omp taskwait

        for(kk=0; kk<k; kk+=2)
            idx[kk/2+1]= idx[kk+2];

        flag = k&1; // remainder
        k = k>>1;
        k += flag;
        if (flag) idx[k]=idx[2*k-1]; 
    }
    
    delete temp;
}

//////////////////////// INSERTION SORT ////////////////////////
void insertionSort(int *generation_rank, int *generation_cost, int high){
    int i,j,key,key_idx; 
    for (i=1; i<=high; ++i){
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