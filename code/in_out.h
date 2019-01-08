/**
in_out.h
Purpose: expone in/out utilities for gen_tsp.cpp

@author Danilo Franco
*/

#include <iostream>
#include <fstream>

using namespace std;
/**
Prints a contiguous memory zone on the standard output (matrix of rows*cols elements)

@param  matrix: Pointer to the first element
@param  rows: Number of rows in the matrix form
@param  cols: Number of columns in the matrix form
*/
void printMatrix(int *matrix, int rows, int cols){
    for (int i=0; i<rows; ++i){
        cout<<endl;
        for(int j=0; j<cols; ++j)
            cout << matrix[cols*i+j] << '\t';    
    }
    cout<<endl<<endl;
    return;
}

/**
Reads from a file of three values per line (xPos yPos Val) and stores them accordingly

@param  cost_matrix: Pointer to the first element of contiguous memory to be written
@param  input_f: Filename
@param  cols: Number of columns in the matrix form of cost_matrix
*/
void readHeatMat(int *cost_matrix, const char *input_f, int cols){
    char row[10],col[10],val[20];
    ifstream myFileStream(input_f);
    if (myFileStream.is_open())
        while (myFileStream >> row >> col >> val){
            cost_matrix[atoi(col)+cols*atoi(row)] = atoi(val);
            cost_matrix[atoi(row)+cols*atoi(col)] = atoi(val);
        }
        return;
}