#include <iostream>
#include <ctime>
#include <chrono>
#include <fstream>
#include <cmath>
#include <algorithm>  //for random_shuffle http://cplusplus.com/reference/algorithm/random_shuffle/
#include <set>

using namespace std;


void printMatrix(int *matrix, int rows, int cols){
    for (int i=0; i<rows; ++i){
        cout<<endl;
        for(int j=0; j<cols; ++j)
            cout << matrix[cols*i+j] << '\t';    
    }
    cout<<endl<<endl;
    return;
}

void readHeatMat(int *cost_matrix, const char *input_f, int numNodes){
    char row[10],col[10],val[20];
    ifstream myFileStream(input_f);
    if (myFileStream.is_open())
        while (myFileStream >> row >> col >> val){
            cost_matrix[atoi(col)+numNodes*atoi(row)] = atoi(val);
            cost_matrix[atoi(row)+numNodes*atoi(col)] = atoi(val);
        }
        return;
}